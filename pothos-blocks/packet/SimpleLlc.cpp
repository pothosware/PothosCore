// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <list>
#include <cstring>
#include <Pothos/Framework.hpp>
#include "MacHelper.hpp"

/***********************************************************************
 * |PothosDoc Simple LLC
 *
 * This LLC is a simple implementation of a Logic Link Control
 * supporting a retransmission of lost data
 * http://en.wikipedia.org/wiki/Logical_link_control
 *
 * |category /Packet
 * |keywords LLC
 *
 * |param Port The port id this LLC is going to be servicing
 * |default 0
 *
 * |param Recipient The ID of the recipient that the LLC is conneting to 
 * |default 0
 *
 * |param ResendTime The time in seconds that is required to be waited before re-sending the outgoing packet
 * |default 1.0
 *
 * |factory /blocks/simple_llc()
 * |setter setPort(Port)
 * |setter setRecipient(Recipient)
 * |setter setResendTime(ResendTime)
 **********************************************************************/
class SimpleLlc : public Pothos::Block
{
    static const uint8_t SEND = 0x1;
    static const uint8_t ACK = 0x2;
    static const uint8_t REQ_ACK = 0x4;
    static const uint8_t RESET = 0x8;
public:
    SimpleLlc(void):
        _errorCount(0), _port(0), _recipient(0), _lastNonceSent(0), _expectedRecvNonce(0), _resendTime(1.0), _resetState(true)
    {
        this->setupInput("macIn");
        this->setupInput("streamIn");
        this->setupOutput("macOut");
        this->setupOutput("streamOut");
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
        _macIn = this->input("macIn");
        _streamIn = this->input("streamIn");
        _macOut = this->output("macOut");
        _streamOut = this->output("streamOut");
        
        // Initialize some frequently used structures
        _resetBeacon.packet.payload = Pothos::BufferChunk("uint8", 4);
        fillHeader(_resetBeacon.packet.payload.as<uint8_t *>(), 0, RESET | REQ_ACK);
        _resetBeacon.packet.metadata["recipient"] = Pothos::Object(_recipient);
        _resetBeacon.sendTime = std::chrono::high_resolution_clock::now();
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
            std::chrono::duration<double> timeSinceSent = timeNow - _resetBeacon.sendTime;
            if(timeSinceSent.count() > _resendTime)
            {
                _resetBeacon.sendTime = timeNow;
                _macOut->postMessage(_resetBeacon.packet);
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
                _streamOut->postMessage(pkt);
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
        if (_streamIn->hasMessage())
        {
            if(!safeToQueueNewPacket()) return;

            auto msg = _streamIn->popMessage();
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
        // Data byte format: RECIPIENT_PORT NONCE_MSC NONCE_LSB CONTROL [DATA]*
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
        uint8_t nonce;
        Pothos::Packet packet;
        std::chrono::high_resolution_clock::time_point sendTime;
        bool sent;
    };
    
    unsigned long long _errorCount;
    uint8_t _port;
    uint8_t _recipient;
    uint16_t _lastNonceSent;
    uint16_t _expectedRecvNonce;
    double _resendTime;
    
    bool _resetState;
    PacketItem _resetBeacon;
    
    std::mutex _lock;
    std::list<PacketItem> _sentPackets;
    
    Pothos::OutputPort *_macOut;
    Pothos::OutputPort *_streamOut;
    Pothos::InputPort *_macIn;
    Pothos::InputPort *_streamIn;
};

static Pothos::BlockRegistry registerSimpleLlc(
    "/blocks/simple_llc", &SimpleLlc::make);
