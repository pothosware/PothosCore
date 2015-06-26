// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <mutex>
#include <chrono>
#include <list>
#include <cstring>

/***********************************************************************
 * |PothosDoc Simple LLC
 *
 * The Simple LLC block is a simple implementation of a logic link control
 * that supports the retransmission of lost data.
 * This block is intended to be used with the simple MAC
 * to interface between user data and the lower layer MAC.
 * The LLC uses imposes an additional packet header over the MAC layer
 * to monitor packet flow and to implement control communication.
 *
 * http://en.wikipedia.org/wiki/Logical_link_control
 *
 * <h3>Ports</h3>
 * Multiple LLC blocks can be connected to a single MAC block,
 * using the port number to differentiate between data channels.
 * The port number is used for both source and destination addressing.
 * Communicating pairs of LLC blocks should use the same port number.
 *
 * <h2>Interfaces</h2>
 * The Simple LLC block has 4 ports that operate on packet streams:
 * <ul>
 * <li><b>dataIn</b> - This port accepts a packet of user data.</li>
 * <li><b>macOut</b> - This port produces a packet intended for the macIn port on the Simple MAC block.
 *  This packet may contain user data or control data with an additional LLC header appended.
 *  The packet metadata has the "recipient" field set to the remote destination MAC.</li>
 * <li><b>macIn</b> - This port accepts a packet from the macOut port on the Simple MAC block.
 *  The LLC header is inspected for destination port, control information, and user data.</li>
 * <li><b>dataOut</b> - This port produces a packet of user data.</li>
 * </ul>
 *
 * |category /Packet
 * |keywords LLC MAC packet
 *
 * |param port The port number that this LLC will servicing.
 * The port number should match the port of the remote LLC.
 * |default 0
 *
 * |param recipient The 16-bit ID of the remote destination MAC.
 * |default 0
 *
 * |param resendTime[Resend Time] Required wait time in seconds before re-sending the outgoing packet.
 * |default 1.0
 * |units seconds
 *
 * |factory /blocks/simple_llc()
 * |setter setPort(port)
 * |setter setRecipient(recipient)
 * |setter setResendTime(resendTime)
 **********************************************************************/
class SimpleLlc : public Pothos::Block
{
    static const uint8_t SEND = 0x1;
    static const uint8_t ACK = 0x2;
    static const uint8_t REQ_ACK = 0x4;
    static const uint8_t RESET = 0x8;
public:
    SimpleLlc(void):
        _errorCount(0), _port(0), _recipient(0), _lastNonceSent(0xFFFF), _expectedRecvNonce(0), _resendTime(1.0), _resetState(true)
    {
        this->setupInput("macIn");
        this->setupInput("dataIn");
        this->setupOutput("macOut");
        this->setupOutput("dataOut");
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setPort));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setRecipient));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setResendTime));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, onUpdateTick));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setResetState));
    }

    static Block *make(void)
    {
        return new SimpleLlc();
    }

    void activate(void)
    {
        this->setResetState(false);
        _macIn = this->input("macIn");
        _dataIn = this->input("dataIn");
        _macOut = this->output("macOut");
        _dataOut = this->output("dataOut");
    }

    void deactivate(void)
    {
        this->setResetState(true);
    }

    void setRecipient(uint16_t recipient)
    {
        _recipient = recipient;
    }

    void setPort(uint16_t port)
    {
        _port = port;
    }

    void setResendTime(double time)
    {
        _resendTime = time;
    }

    void setResetState(bool resetState)
    {
        _resetState = resetState;
    }

    void onUpdateTick(int /*dummy*/)
    {
        auto timeNow = std::chrono::high_resolution_clock::now();

        // While in reset state, send out reset beacons regularly
        if(_resetState)
        {
            std::chrono::duration<double> timeSinceSent = timeNow - _resetBeaconSendTime;
            if(timeSinceSent.count() > _resendTime)
            {
                _resetBeaconSendTime = timeNow;
                uint16_t nonce = 0;
                if(!_sentPackets.empty()) nonce = _sentPackets.front().nonce;

                postControlPacket(nonce, RESET | REQ_ACK);
            }
        }

        if(_resetState) return;

        if(!_lock.try_lock()) return;

        for(auto &item: _sentPackets)
        {
            if(!item.sent) continue;

            std::chrono::duration<double> timeSinceSent = timeNow - item.sendTime;
            // When we reach an item that has not been timed out yet, we know all the
            // packets were queued later, therefore wouldn't be timed out either.
            if(timeSinceSent.count() < _resendTime) break;

            item.sendTime = timeNow;
            _macOut->postMessage(item.packet);
        }

        _lock.unlock();
    }

    void postControlPacket(uint16_t nonce, uint8_t control)
    {
        //FIXME: Save the previously sent ack packet and use the .unique() to check if
        // that previous packet could be reused, so as to avoid reallocation of buffer space that happens below
        Pothos::Packet packet;
        packet.payload = Pothos::BufferChunk("uint8", 4);
        fillHeader(packet.payload.as<uint8_t *>(), nonce, control);
        packet.metadata["recipient"] = Pothos::Object(_recipient);
        _macOut->postMessage(packet);
    }

    void work(void)
    {
        // handle incoming data from MAC
        if (_macIn->hasMessage())
        {
            auto msg = _macIn->popMessage();
            auto pkt = msg.extract<Pothos::Packet>();

            if (pkt.payload.length < 4) return;
            const auto byteBuf = pkt.payload.as<const uint8_t *>();

            uint8_t port;
            uint16_t nonce;
            uint8_t control;
            extractHeader(byteBuf, port, nonce, control);

            if(port != _port) return;

            // Initiate the reset if the other side requests a reset, and finish the reset if we receive a reset acknowledgment
            if(control & RESET)
            {
                if(control & ACK)
                    _resetState = false;
                else
                {
                    _resetState = true;
                    _expectedRecvNonce = nonce;
                }
            }

            // Responding with the acknowledgment
            if((control & REQ_ACK) && _expectedRecvNonce == nonce)
            {
                postControlPacket(nonce, (control & ~REQ_ACK) | ACK);
            }

            // Posting the incoming data forward
            if((control & SEND) && !(control & ACK) && _expectedRecvNonce == nonce)
            {
                pkt.payload.address += 4;
                pkt.payload.length -= 4;
                _dataOut->postMessage(pkt);
                _expectedRecvNonce++;
            }

            // Remove the acknowledged packet from being re-sent again
            if((control & ACK) && (control & SEND))
            {
                _lock.lock();
                for(auto iter = _sentPackets.begin(), end = _sentPackets.end(); iter != end; iter++)
                {
                    // TODO: perform an network unitilization and delay optimization trick,
                    // where an acknowledged nonce can be used to determine that all packets
                    // with nonce < received_nonce were all successfully sent.
                    // Useful in case an ack packet is lost in transmission.
                    // Trick requires a little bit more careful nonce handling in other parts of the code.
                    if((*iter).nonce == nonce)
                    {
                        _sentPackets.erase(iter);
                        break;
                    }
                };
                _lock.unlock();
            }
        }

        // handle outgoing data to MAC
        if (_dataIn->hasMessage())
        {
            if(!safeToQueueNewPacket()) return;

            auto msg = _dataIn->popMessage();
            auto pktIn = msg.extract<Pothos::Packet>();
            auto data = pktIn.payload;

            _lock.lock();
            uint16_t nonce = ++_lastNonceSent;
            PacketItem &item = *_sentPackets.insert(_sentPackets.end(), PacketItem());
            item.sent = false;
            _lock.unlock();

            item.sendTime = std::chrono::high_resolution_clock::now();

            item.nonce = nonce;
            item.packet.payload = Pothos::BufferChunk("uint8", data.length + 4);
            auto byteBuf = item.packet.payload.as<uint8_t *>();
            fillHeader(byteBuf, nonce, SEND | REQ_ACK);
            std::memcpy(byteBuf + 4, data.as<const uint8_t*>(), data.length);

            item.packet.metadata["recipient"] = Pothos::Object(_recipient);

            _macOut->postMessage(item.packet);
            item.sent = true;
        }
    }

private:
    void fillHeader(uint8_t *byteBuf, uint16_t nonce, uint8_t control)
    {
        // Data byte format: RECIPIENT_PORT NONCE_MSB NONCE_LSB CONTROL [DATA]*
        byteBuf[0] = _port;
        byteBuf[1] = nonce >> 8;
        byteBuf[2] = nonce % 256;
        byteBuf[3] = control;
    }

    void extractHeader(const uint8_t *byteBuf, uint8_t &port, uint16_t &nonce, uint8_t &control)
    {
        // Data byte format: RECIPIENT_PORT NONCE_MSB NONCE_LSB CONTROL [DATA]*
        port = byteBuf[0];
        nonce = (byteBuf[1] << 8) | byteBuf[2];
        control = byteBuf[3];
    }

    bool safeToQueueNewPacket()
    {
        bool safe = true;

        if(_resetState) return false;

        _lock.lock();
        auto &item = _sentPackets.front();
        if(_lastNonceSent + 1 == item.nonce) safe = false;
        _lock.unlock();

        return safe;
    }

    struct PacketItem
    {
        uint16_t nonce;
        Pothos::Packet packet;
        std::chrono::high_resolution_clock::time_point sendTime;
        bool sent;
    };

    unsigned long long _errorCount;
    uint8_t _port;
    uint16_t _recipient;
    uint16_t _lastNonceSent;
    uint16_t _expectedRecvNonce;
    double _resendTime;

    bool _resetState;
    std::chrono::high_resolution_clock::time_point _resetBeaconSendTime;

    std::mutex _lock;
    std::list<PacketItem> _sentPackets;

    Pothos::OutputPort *_macOut;
    Pothos::OutputPort *_dataOut;
    Pothos::InputPort *_macIn;
    Pothos::InputPort *_dataIn;
};

static Pothos::BlockRegistry registerSimpleLlc(
    "/blocks/simple_llc", &SimpleLlc::make);
