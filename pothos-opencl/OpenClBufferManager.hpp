// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Framework/BufferManager.hpp>
#include <memory>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

struct OpenClBufferContainerArgs
{
    cl_mem_flags mem_flags;
    cl_map_flags map_flags;
    std::shared_ptr<cl_context> context;
    std::shared_ptr<cl_command_queue> queue;
};

Pothos::BufferManager::Sptr makeOpenClBufferManager(const OpenClBufferContainerArgs &);

cl_mem getClBufferFromManaged(const Pothos::ManagedBuffer &buff);
