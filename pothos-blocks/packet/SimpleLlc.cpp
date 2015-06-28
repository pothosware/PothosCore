// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <Pothos/Util/SpinLock.hpp>
#include <thread>
#include <mutex> //lock_guard
#include <chrono>
#include <iostream>
#include <cstdint>
#include <cstring> //memcpy

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
 * http://en.wikipedia.org/wiki/Go-Back-N_ARQ
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
 * |param port[Port Number] The port number that this LLC will servicing.
 * The port number is 8-bits and should match the port of the remote LLC.
 * |default 0
 *
 * |param recipient[Recipient ID] The 16-bit ID of the remote destination MAC.
 * |default 0
 *
 * |param resendTimeout[Resend Timeout] Timeout in seconds before re-sending the outgoing packet.
 * The LLC will resend any packets that have not been acknowledged within this time window.
 * |default 0.01
 * |units seconds
 *
 * |param expireTimeout[Expire Timeout] Maximum time in seconds that LLC can hold a packet.
 * The LLC will try to guarantee delivery of a packet by resending within this time window.
 * |default 0.1
 * |units seconds
 *
 * |param windowSize[Window Size] The number of packets allowed out before an acknowledgment is required.
 * |default 4
 *
 * |factory /blocks/simple_llc()
 * |setter setPort(port)
 * |setter setRecipient(recipient)
 * |setter setResendTimeout(resendTimeout)
 * |setter setExpireTimeout(expireTimeout)
 * |setter setWindowSize(windowSize)
 **********************************************************************/
class SimpleLlc : public Pothos::Block
{
    static const uint8_t PSH = 0x1; //push data packet type
    static const uint8_t REQ = 0x4; //request packet type
    static const uint8_t SYN = 0x8; //synchronize sequence
public:
    SimpleLlc(void):
        _resendCount(0),
        _expiredCount(0),
        _port(0),
        _recipient(0),
        _windowSize(0),
        _seqBase(0),
        _seqOut(0),
        _reqSeq(0),
        _resendMsg(1)
    {
        this->setupInput("macIn");
        this->setupInput("dataIn");
        this->setupOutput("macOut");
        this->setupOutput("dataOut");
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setPort));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setRecipient));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setResendTimeout));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setExpireTimeout));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, setWindowSize));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, getResendCount));
        this->registerCall(this, POTHOS_FCN_TUPLE(SimpleLlc, getExpiredCount));
        this->registerProbe("getResendCount");
        this->registerProbe("getExpiredCount");
        this->setWindowSize(4); //initial state
        this->setRecipient(0); //initial state
        this->setResendTimeout(0.01); //initial state
        this->setExpireTimeout(0.1); //initial state
    }

    static Block *make(void)
    {
        return new SimpleLlc();
    }

    void activate(void)
    {
        //we must be able to synchronize to any random starting sequence
        _reqSeq = std::rand() & 0xffff;
        _seqBase = std::rand() & 0xffff;
        _seqOut = _seqBase;

        //grab pointers to the ports
        _macIn = this->input("macIn");
        _dataIn = this->input("dataIn");
        _macOut = this->output("macOut");
        _dataOut = this->output("dataOut");

        //start the monitor thread
        _monitorThread = std::thread(&SimpleLlc::monitorTimeoutsTask, this);
    }

    void deactivate(void)
    {
        _monitorThread.join();
    }

    void monitorTimeoutsTask(void)
    {
        while (this->isActive())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            const auto timeNow = std::chrono::high_resolution_clock::now();

            std::lock_guard<Pothos::Util::SpinLock> lock(_lock);

            //remove expired packets, oldest to newest
            while (not _sentPackets.empty() and _sentPackets.front().expiredTime < timeNow)
            {
                _sentPackets.pop_front();
                _seqBase++;
                _expiredCount++;
            }

            //check if the oldest packet should cause full resend
            if (_sentPackets.empty()) continue;
            const auto delta = timeNow - _sentPackets.front().lastSentTime;
            if (delta > _resendTimeout) _macIn->pushMessage(_resendMsg);
        }
    }

    void setRecipient(const uint16_t recipient)
    {
        _recipient = recipient;
        _metadata["recipient"] = Pothos::Object(_recipient);
    }

    void setPort(const uint16_t port)
    {
        _port = port;
    }

    void setResendTimeout(const double timeout)
    {
        _resendTimeout = std::chrono::nanoseconds(long(timeout*1e9));
    }

    void setExpireTimeout(const double timeout)
    {
        _expireTimeout = std::chrono::nanoseconds(long(timeout*1e9));
    }

    void setWindowSize(const size_t windowSize)
    {
        _windowSize = windowSize;
        _sentPackets.set_capacity(_windowSize);
    }

    unsigned long long getResendCount(void) const
    {
        return _resendCount;
    }

    unsigned long long getExpiredCount(void) const
    {
        return _expiredCount;
    }

    void work(void)
    {
        // handle incoming data from MAC
        while (_macIn->hasMessage())
        {
            auto msg = _macIn->popMessage();

            //handle the resend message from the timeout monitor thread
            if (msg == _resendMsg)
            {
                this->resendPackets();
                continue;
            }

            //extract the packet
            auto pkt = msg.extract<Pothos::Packet>();
            if (pkt.payload.length < 4) continue;
            const auto byteBuf = pkt.payload.as<const uint8_t *>();

            //parse the header
            uint8_t port = 0;
            uint16_t nonce = 0;
            uint8_t control = 0;
            extractHeader(byteBuf, port, nonce, control);

            //was this packet intended for this LLC?
            if(port != _port) continue;

            //got a synchronize packet from sender
            if ((control & SYN) != 0) _reqSeq = nonce;

            //got a datagram packet from sender
            if((control & PSH) != 0)
            {
                //got the expected sequence, forward the packet
                if (nonce == _reqSeq)
                {
                    pkt.payload.address += 4;
                    pkt.payload.length -= 4;
                    _dataOut->postMessage(pkt);
                    _reqSeq++;
                }

                //always reply with a request
                postControlPacket(_reqSeq, REQ);
            }

            //got a request packet from receiver
            if((control & REQ) != 0)
            {
                std::lock_guard<Pothos::Util::SpinLock> lock(_lock);

                //check for sequence obviously out of range and request resync
                if (nonce < _seqBase or nonce > _seqOut)
                {
                    this->postControlPacket(_seqBase, SYN);
                }

                //otherwise clear everything sent up to but not including the latest request
                else for (; _seqBase < nonce; _seqBase++)
                {
                    if (not _sentPackets.empty()) _sentPackets.pop_front();
                }
            }
        }

        // return without handling the user data if we are flow controlled
        {
            std::lock_guard<Pothos::Util::SpinLock> lock(_lock);
            if (_sentPackets.full()) return;
        }

        // handle outgoing data to MAC
        while (_dataIn->hasMessage())
        {
            //extract the packet
            auto msg = _dataIn->popMessage();
            auto pktIn = msg.extract<Pothos::Packet>();
            auto data = pktIn.payload;

            //append the LLC header
            Pothos::Packet pktOut = pktIn;
            pktOut.metadata = _metadata;
            pktOut.payload = Pothos::BufferChunk(data.length + 4);
            pktOut.payload.dtype = pktIn.payload.dtype;
            auto byteBuf = pktOut.payload.as<uint8_t *>();
            fillHeader(byteBuf, _seqOut++, PSH);
            std::memcpy(byteBuf + 4, data.as<const uint8_t*>(), data.length);
            _macOut->postMessage(pktOut);

            //save the packet for resending
            PacketItem item;
            item.packet = pktOut;
            const auto timeNow = std::chrono::high_resolution_clock::now();
            item.lastSentTime = timeNow;
            item.expiredTime = timeNow + _expireTimeout;

            //stash the packet and check capacity (locked)
            std::lock_guard<Pothos::Util::SpinLock> lock(_lock);
            _sentPackets.push_back(item);
            if (_sentPackets.full()) break;
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

    void postControlPacket(uint16_t nonce, uint8_t control)
    {
        //FIXME: Save the previously sent ack packet and use the .unique() to check if
        // that previous packet could be reused, so as to avoid reallocation of buffer space that happens below
        Pothos::Packet packet;
        packet.metadata = _metadata;
        packet.payload = Pothos::BufferChunk(4);
        fillHeader(packet.payload.as<uint8_t *>(), nonce, control);
        _macOut->postMessage(packet);
    }

    void resendPackets(void)
    {
        const auto timeNow = std::chrono::high_resolution_clock::now();
        std::lock_guard<Pothos::Util::SpinLock> lock(_lock);
        for (size_t i = 0; i < _sentPackets.size(); i++)
        {
            _macOut->postMessage(_sentPackets[i].packet);
            _sentPackets[i].lastSentTime = timeNow;
            _resendCount++;
        }
    }

    struct PacketItem
    {
        Pothos::Packet packet;
        std::chrono::high_resolution_clock::time_point expiredTime; //used for expiration
        std::chrono::high_resolution_clock::time_point lastSentTime; //used for resending
    };

    //status counts
    unsigned long long _resendCount;
    unsigned long long _expiredCount;

    //configuration
    uint8_t _port;
    uint16_t _recipient;
    Pothos::ObjectKwargs _metadata;
    std::chrono::high_resolution_clock::duration _resendTimeout;
    std::chrono::high_resolution_clock::duration _expireTimeout;
    uint16_t _windowSize;

    //sender side state
    Pothos::Util::SpinLock _lock;
    Pothos::Util::RingDeque<PacketItem> _sentPackets;
    uint16_t _seqBase;
    uint16_t _seqOut;

    //receiver side state
    uint16_t _reqSeq;

    std::thread _monitorThread;
    const Pothos::Object _resendMsg;

    //pointers for port access
    Pothos::OutputPort *_macOut;
    Pothos::OutputPort *_dataOut;
    Pothos::InputPort *_macIn;
    Pothos::InputPort *_dataIn;
};

static Pothos::BlockRegistry registerSimpleLlc(
    "/blocks/simple_llc", &SimpleLlc::make);
