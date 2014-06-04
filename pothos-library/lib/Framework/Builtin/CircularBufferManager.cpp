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
    CircularBufferManager(void)
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
        _slabIndex = 0;
        _indexToAck = 0;

        //size internal containers
        _readyBuffs.set_capacity(args.numBuffers);
        _pushedBuffers.resize(args.numBuffers);

        //fill the ready queue with available buffers
        for (size_t i = 0; i < args.numBuffers; i++)
        {
            _readyBuffs.push_back(Pothos::ManagedBuffer());
        }

        //setup the front buffer for action
        this->prepareFront();
    }

    bool empty(void) const
    {
        return _readyBuffs.empty();
    }

    const Pothos::ManagedBuffer &front(void) const
    {
        assert(not _readyBuffs.empty());
        return _readyBuffs.front();
    }

    void pop(const size_t numBytes)
    {
        assert(not _readyBuffs.empty());
        _readyBuffs.pop_front();

        //increment front address and adjust for aliasing
        assert(_bufferSize >= numBytes);
        _frontAddress += numBytes;
        if (_frontAddress >= _circBuff.getAddress() + _circBuff.getLength()) _frontAddress -= _circBuff.getLength();

        //increment for the next buffer index
        if (++_slabIndex == _pushedBuffers.size()) _slabIndex = 0;

        //prepare the next buffer in the queue
        this->prepareFront();
    }

    void push(const Pothos::ManagedBuffer &buff)
    {
        //store the buffer into its position
        assert(buff.getSlabIndex() < _pushedBuffers.size());
        _pushedBuffers[buff.getSlabIndex()] = buff;

        //look for pushed buffers -- but in order
        while (_pushedBuffers[_indexToAck])
        {
            //move the buffer into the queue
            assert(not _readyBuffs.full());
            _readyBuffs.push_back(_pushedBuffers[_indexToAck]);
            _pushedBuffers[_indexToAck].reset();

            //increment for the next pushed buffer
            if (++_indexToAck == _pushedBuffers.size()) _indexToAck = 0;
        }

        this->prepareFront();
    }

private:

    void prepareFront(void)
    {
        if (_readyBuffs.empty()) return;

        //setup the front to point to available memory
        Pothos::SharedBuffer buffer(_frontAddress, _bufferSize, _circBuff);
        _readyBuffs.front().reset(this->shared_from_this(), buffer, _slabIndex);
    }

    size_t _frontAddress;
    size_t _bufferSize;
    size_t _slabIndex;
    size_t _indexToAck;
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
