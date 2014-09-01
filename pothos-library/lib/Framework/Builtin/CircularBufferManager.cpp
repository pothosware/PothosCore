// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <vector>
#include <cassert>

/***********************************************************************
 * circular buffer implementation
 **********************************************************************/
class CircularBufferManager :
    public Pothos::BufferManager,
    public std::enable_shared_from_this<CircularBufferManager>
{
public:
    CircularBufferManager(void):
        _frontAddress(0),
        _bufferSize(0),
        _indexToAck(0),
        _bytesToPop(0)
    {
        return;
    }

    void init(const Pothos::BufferManagerArgs &args)
    {
        //create the circular buffer
        _circBuff = Pothos::SharedBuffer::makeCirc(
            args.bufferSize*args.numBuffers, args.nodeAffinity);

        //init the state variables
        _frontAddress = _circBuff.getAddress();
        _bufferSize = args.bufferSize;
        _indexToAck = 0;

        //size internal containers
        _readyBuffs.set_capacity(args.numBuffers);
        _pushedBuffers.resize(args.numBuffers);

        //allocate buffer token objects
        for (size_t i = 0; i < args.numBuffers; i++)
        {
            Pothos::ManagedBuffer buffer;
            buffer.reset(this->shared_from_this(), _circBuff, i/*slabIndex*/);
        }
    }

    bool empty(void) const
    {
        return _readyBuffs.empty();
    }

    void pop(const size_t numBytes)
    {
        assert(not _readyBuffs.empty());
        _bytesToPop += numBytes;

        //re-use the buffer for small consumes
        if (this->front().length >= numBytes*2)
        {
            auto buff = this->front();
            buff.address += numBytes;
            buff.length -= numBytes;
            this->setFrontBuffer(buff);
            return;
        }

        _readyBuffs.pop_front();

        //increment front address and adjust for aliasing
        assert(_bufferSize >= _bytesToPop);
        _frontAddress += _bytesToPop;
        _bytesToPop = 0;
        if (_frontAddress >= _circBuff.getAddress() + _circBuff.getLength()) _frontAddress -= _circBuff.getLength();

        //prepare the next buffer in the queue
        if (_readyBuffs.empty()) this->setFrontBuffer(Pothos::BufferChunk::null());
        else this->setCircFrontBuffer(_readyBuffs.front());
    }

    void push(const Pothos::ManagedBuffer &buff)
    {
        //store the buffer into its position
        assert(buff.getSlabIndex() < _pushedBuffers.size());
        _pushedBuffers[buff.getSlabIndex()] = buff;

        //look for pushed buffers -- but in order
        while (_pushedBuffers[_indexToAck])
        {
            if (_readyBuffs.empty()) this->setCircFrontBuffer(_pushedBuffers[_indexToAck]);

            //move the buffer into the queue
            assert(not _readyBuffs.full());
            _readyBuffs.push_back(_pushedBuffers[_indexToAck]);
            _pushedBuffers[_indexToAck].reset();

            //increment for the next pushed buffer
            if (++_indexToAck == _pushedBuffers.size()) _indexToAck = 0;
        }
    }

private:

    void setCircFrontBuffer(Pothos::ManagedBuffer &buff)
    {
        Pothos::SharedBuffer sbuff(_frontAddress, _bufferSize, _circBuff);
        buff.reset(this->shared_from_this(), sbuff, buff.getSlabIndex());
        this->setFrontBuffer(buff);
    }

    size_t _frontAddress;
    size_t _bufferSize;
    size_t _indexToAck;
    size_t _bytesToPop;
    Pothos::SharedBuffer _circBuff;
    std::vector<Pothos::ManagedBuffer> _pushedBuffers;
    Pothos::Util::RingDeque<Pothos::ManagedBuffer> _readyBuffs;
};

/***********************************************************************
 * factory and registration
 **********************************************************************/
Pothos::BufferManager::Sptr makeCircularBufferManager(void)
{
    return std::make_shared<CircularBufferManager>();
}

pothos_static_block(pothosFrameworkRegisterCircularBufferManager)
{
    Pothos::PluginRegistry::addCall(
        "/framework/buffer_manager/circular",
        &makeCircularBufferManager);
}
