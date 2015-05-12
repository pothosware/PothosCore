// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework.hpp>
#include <Pothos/Util/OrderedQueue.hpp>

class SDRSourceBufferManager :
    public Pothos::BufferManager,
    public std::enable_shared_from_this<SDRSourceBufferManager>
{
public:
    SDRSourceBufferManager(SoapySDR::Device *device, SoapySDR::Stream *stream):
        _device(device),
        _stream(stream)
    {
        return;
    }

    ~SDRSourceBufferManager(void)
    {
        return;
    }

    void init(const Pothos::BufferManagerArgs &)
    {
        Pothos::BufferManagerArgs args;
        args.numBuffers = _device->getNumDirectAccessBuffers(_stream);
        args.bufferSize = _device->getStreamMTU(_stream);
        _readyBuffs = Pothos::Util::RingDeque<Pothos::ManagedBuffer>(args.numBuffers);

        //this will flag the manager as initialized after the allocation above
        Pothos::BufferManager::init(args);

        for (size_t i = 0; i < args.numBuffers; i++)
        {
            size_t handle = 0;
            auto container = std::make_shared<int>(0);
            auto sharedBuff = Pothos::SharedBuffer(size_t(0), args.bufferSize, container);
            Pothos::ManagedBuffer buffer;
            buffer.reset(this->shared_from_this(), sharedBuff, handle);
        }
    }

    bool empty(void) const
    {
        return _readyBuffs.empty();
    }

    void pop(const size_t /*numBytes*/)
    {
        //boiler-plate to pop from the queue and set the front buffer
        assert(not _readyBuffs.empty());
        auto buff = _readyBuffs.front();
        _readyBuffs.pop_front();

        //prepare the next buffer in the queue
        if (_readyBuffs.empty()) this->setFrontBuffer(Pothos::BufferChunk::null());
        else this->setFrontBuffer(_readyBuffs.front());
    }

    void push(const Pothos::ManagedBuffer &buff)
    {
        assert(buff.getSlabIndex() < _readyBuffs.capacity());
        _device->releaseReadBuffer(_stream, buff.getSlabIndex());
        _readyBuffs.push_back(buff);

        //prepare the next buffer in the queue
        if (not _readyBuffs.empty()) this->setFrontBuffer(_readyBuffs.front());
    }

    void updateFront(const size_t handle, const void *addr)
    {
        auto &buff = _readyBuffs.front();
        const auto &sbuff = buff.getBuffer();
        auto sharedBuff = Pothos::SharedBuffer(size_t(addr), sbuff.getLength(), sbuff.getContainer());
        buff.reset(this->shared_from_this(), sharedBuff, handle);
    }

private:
    SoapySDR::Device *_device;
    SoapySDR::Stream *_stream;
    Pothos::Util::RingDeque<Pothos::ManagedBuffer> _readyBuffs;
};
