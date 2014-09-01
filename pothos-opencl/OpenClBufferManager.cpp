// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "OpenClKernel.hpp"
#include <Pothos/Plugin.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <cassert>
#include <iostream>

/***********************************************************************
 * The OpenClBufferContainer allocates and maps a buffer.
 * It knows how to cleanup when the buffer dereferences.
 **********************************************************************/
class OpenClBufferContainer
{
public:
    OpenClBufferContainer(const OpenClBufferContainerArgs &clArgs, const size_t bufferSize):
        _clArgs(clArgs)
    {
        cl_int err = 0;
        memobj = clCreateBuffer(*_clArgs.context, _clArgs.mem_flags, bufferSize, nullptr, &err);
        if (err < 0) throw Pothos::Exception("OpenClBufferContainer::clCreateBuffer()", clErrToStr(err));
        mapped_ptr = clEnqueueMapBuffer(
            *_clArgs.queue,
            memobj,
            CL_TRUE, /*blocking map*/
            _clArgs.map_flags,
            0, //offset
            bufferSize,
            0, nullptr, nullptr,
            &err);
        if (err < 0) throw Pothos::Exception("OpenClBufferContainer::clEnqueueMapBuffer()", clErrToStr(err));
    }

    ~OpenClBufferContainer(void)
    {
        clEnqueueUnmapMemObject(*_clArgs.queue, memobj, mapped_ptr, 0, nullptr, nullptr);
        clReleaseMemObject(memobj);
    }

    void *mapped_ptr;
    cl_mem memobj;

private:
    OpenClBufferContainerArgs _clArgs;
};

/***********************************************************************
 * Custom buffer manager for dealing with opencl mapped buffers
 **********************************************************************/
class OpenClBufferManager :
    public Pothos::BufferManager,
    public std::enable_shared_from_this<OpenClBufferManager>
{
public:
    OpenClBufferManager(const OpenClBufferContainerArgs &clArgs):
        _clArgs(clArgs)
    {
        return;
    }

    void init(const Pothos::BufferManagerArgs &args)
    {
        _readyBuffs.set_capacity(args.numBuffers);
        for (size_t i = 0; i < args.numBuffers; i++)
        {
            auto container = std::make_shared<OpenClBufferContainer>(_clArgs, args.bufferSize);
            auto sharedBuff = Pothos::SharedBuffer(size_t(container->mapped_ptr), args.bufferSize, container);
            Pothos::ManagedBuffer buffer;
            buffer.reset(this->shared_from_this(), sharedBuff);
        }
    }

    bool empty(void) const
    {
        return _readyBuffs.empty();
    }

    void pop(const size_t numBytes)
    {
        assert(not _readyBuffs.empty());
        auto buff = _readyBuffs.front();
        _readyBuffs.pop_front();
        if (not _readyBuffs.empty()) this->setFrontBuffer(_readyBuffs.front());
        else this->setFrontBuffer(Pothos::BufferChunk::null());

        auto container = std::static_pointer_cast<OpenClBufferContainer>(buff.getBuffer().getContainer());
        assert(container);

        //perform non blocking write
        //kernel will be enqueued after this
        if (_clArgs.map_flags == CL_MAP_WRITE)
        {
            const cl_int err = clEnqueueWriteBuffer(
                *_clArgs.queue,
                container->memobj, CL_FALSE, 0,
                numBytes, container->mapped_ptr,
                0, nullptr, nullptr
            );
            if (err < 0) throw Pothos::Exception("OpenClBufferManager::clEnqueueWriteBuffer()", clErrToStr(err));
        }

        //perform blocking read
        //must block before giving downstream memory
        if (_clArgs.map_flags == CL_MAP_READ)
        {
            const cl_int err = clEnqueueReadBuffer(
                *_clArgs.queue,
                container->memobj, CL_TRUE, 0,
                numBytes, container->mapped_ptr,
                0, nullptr, nullptr
            );
            if (err < 0) throw Pothos::Exception("OpenClBufferManager::clEnqueueReadBuffer()", clErrToStr(err));
        }
    }

    void push(const Pothos::ManagedBuffer &buff)
    {
        if (_readyBuffs.empty()) this->setFrontBuffer(buff);
        auto container = std::static_pointer_cast<OpenClBufferContainer>(buff.getBuffer().getContainer());
        assert(container);
        assert(not _readyBuffs.full());
        _readyBuffs.push_back(buff);
    }

private:
    Pothos::Util::RingDeque<Pothos::ManagedBuffer> _readyBuffs;
    OpenClBufferContainerArgs _clArgs;
};

Pothos::BufferManager::Sptr makeOpenClBufferManager(const OpenClBufferContainerArgs &args)
{
    return std::make_shared<OpenClBufferManager>(args);
}

cl_mem &getClBufferFromManaged(const Pothos::ManagedBuffer &buff)
{
    return std::static_pointer_cast<OpenClBufferContainer>(buff.getBuffer().getContainer())->memobj;
}
