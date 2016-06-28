// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Util/OrderedQueue.hpp>
#include <Pothos/Framework/BufferManager.hpp>
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
        _bytesToPop(0)
    {
        return;
    }

    void init(const Pothos::BufferManagerArgs &args)
    {
        Pothos::BufferManager::init(args);

        //create the circular buffer
        _circBuff = Pothos::SharedBuffer::makeCirc(
            args.bufferSize*args.numBuffers, args.nodeAffinity);

        //init the state variables
        _frontAddress = _circBuff.getAddress();
        _bufferSize = args.bufferSize;

        //size internal containers
        _readyBuffs = Pothos::Util::OrderedQueue<Pothos::ManagedBuffer>(args.numBuffers);

        //allocate buffer token objects
        std::vector<Pothos::ManagedBuffer> managedBuffers(args.numBuffers);
        for (size_t i = 0; i < args.numBuffers; i++)
        {
            Pothos::ManagedBuffer buffer;
            managedBuffers[i].reset(this->shared_from_this(), _circBuff, i/*slabIndex*/);
        }

        //set the next buffer pointers
        for (size_t i = 0; i < managedBuffers.size()-1; i++)
        {
            managedBuffers[i].setNextBuffer(managedBuffers[i+1]);
        }
        managedBuffers.back().setNextBuffer(managedBuffers.front());
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
        if (_bytesToPop*2 < _bufferSize)
        {
            auto buff = this->front();
            buff.address += numBytes;
            buff.length -= numBytes;
            this->setFrontBuffer(buff);
            return;
        }

        _readyBuffs.pop();

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
        assert(buff.getSlabIndex() < _readyBuffs.capacity());

        const bool wasEmpty = _readyBuffs.empty();
        _readyBuffs.push(buff, buff.getSlabIndex());

        //prepare the next buffer in the queue
        if (not _readyBuffs.empty() and wasEmpty) this->setCircFrontBuffer(_readyBuffs.front());
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
    size_t _bytesToPop;
    Pothos::SharedBuffer _circBuff;
    Pothos::Util::OrderedQueue<Pothos::ManagedBuffer> _readyBuffs;
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
