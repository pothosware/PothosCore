// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Util/OrderedQueue.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <cassert>
#include <iostream>

/***********************************************************************
 * generic buffer implementation
 **********************************************************************/
class GenericBufferManager :
    public Pothos::BufferManager,
    public std::enable_shared_from_this<GenericBufferManager>
{
public:
    GenericBufferManager(void):
        _bufferSize(0),
        _bytesPopped(0)
    {
        return;
    }

    void init(const Pothos::BufferManagerArgs &args)
    {
        Pothos::BufferManager::init(args);
        _bufferSize = args.bufferSize;
        _readyBuffs = Pothos::Util::OrderedQueue<Pothos::ManagedBuffer>(args.numBuffers);

        //allocate one large continuous slab
        auto commonSlab = Pothos::SharedBuffer::make(
            args.bufferSize*args.numBuffers, args.nodeAffinity);

        //create managed buffers based on chunks from the slab
        std::vector<Pothos::ManagedBuffer> managedBuffers(args.numBuffers);
        for (size_t i = 0; i < args.numBuffers; i++)
        {
            const size_t addr = commonSlab.getAddress()+(args.bufferSize*i);
            Pothos::SharedBuffer sharedBuff(addr, args.bufferSize, commonSlab);
            managedBuffers[i].reset(this->shared_from_this(), sharedBuff, i/*slabIndex*/);
            this->push(managedBuffers[i]);
        }

        //set the next buffer pointers
        for (size_t i = 0; i < managedBuffers.size()-1; i++)
        {
            managedBuffers[i].setNextBuffer(managedBuffers[i+1]);
        }
    }

    bool empty(void) const
    {
        return _readyBuffs.empty();
    }

    void pop(const size_t numBytes)
    {
        assert(not _readyBuffs.empty());
        _bytesPopped += numBytes;

        //re-use the buffer for small consumes
        if (_bytesPopped*2 < _bufferSize)
        {
            auto buff = this->front();
            buff.address += numBytes;
            buff.length -= numBytes;
            this->setFrontBuffer(buff);
            return;
        }

        _bytesPopped = 0;
        _readyBuffs.pop();
        if (_readyBuffs.empty()) this->setFrontBuffer(Pothos::BufferChunk::null());
        else this->setFrontBuffer(_readyBuffs.front());
    }

    void push(const Pothos::ManagedBuffer &buff)
    {
        if (_readyBuffs.empty()) this->setFrontBuffer(buff);
        _readyBuffs.push(buff, buff.getSlabIndex());
    }

private:

    size_t _bufferSize;
    size_t _bytesPopped;
    Pothos::Util::OrderedQueue<Pothos::ManagedBuffer> _readyBuffs;
};

/***********************************************************************
 * factory and registration
 **********************************************************************/
Pothos::BufferManager::Sptr makeGenericBufferManager(void)
{
    return std::make_shared<GenericBufferManager>();
}

pothos_static_block(pothosFrameworkRegisterGenericBufferManager)
{
    Pothos::PluginRegistry::addCall(
        "/framework/buffer_manager/generic",
        &makeGenericBufferManager);
}
