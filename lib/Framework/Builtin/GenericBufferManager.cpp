// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <cassert>

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
        _readyBuffs.set_capacity(args.numBuffers);
        for (size_t i = 0; i < args.numBuffers; i++)
        {
            auto sharedBuff = Pothos::SharedBuffer::make(
                args.bufferSize, args.nodeAffinity);
            Pothos::ManagedBuffer buffer;
            buffer.reset(this->shared_from_this(), sharedBuff, i/*slabIndex*/);
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
        _readyBuffs.pop_front();
        if (_readyBuffs.empty()) this->setFrontBuffer(Pothos::BufferChunk::null());
        else this->setFrontBuffer(_readyBuffs.front());
    }

    void push(const Pothos::ManagedBuffer &buff)
    {
        if (_readyBuffs.empty()) this->setFrontBuffer(buff);
        assert(not _readyBuffs.full());
        _readyBuffs.push_back(buff);
    }

private:

    size_t _bufferSize;
    size_t _bytesPopped;
    Pothos::Util::RingDeque<Pothos::ManagedBuffer> _readyBuffs;
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
