// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "OpenClKernel.hpp"
#include <Pothos/Exception.hpp>
#include <mutex>
#include <map>

std::shared_ptr<cl_context> lookupContextCache(cl_device_id device)
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    static std::map<cl_device_id, std::weak_ptr<cl_context>> contextCache;
    auto &weakContext = contextCache[device];
    auto contextSptr = weakContext.lock();
    if (not contextSptr)
    {
        cl_int err = 0;
        auto context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
        if (err < 0) throw Pothos::Exception("OpenClKernel::clCreateContext()", clErrToStr(err));
        contextSptr.reset(new cl_context(context), &clReleaseContextPtr);
    }
    weakContext = contextSptr;
    return contextSptr;
}
