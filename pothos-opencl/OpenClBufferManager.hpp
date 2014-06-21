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

//! error code number to string
const char *clErrToStr(cl_int err);

/***********************************************************************
 * arguments required to create a custom cl buffer manager
 **********************************************************************/
struct OpenClBufferContainerArgs
{
    cl_mem_flags mem_flags;
    cl_map_flags map_flags;
    std::shared_ptr<cl_context> context;
    std::shared_ptr<cl_command_queue> queue;
};

//! Factory function for creating a cl buffer manager
Pothos::BufferManager::Sptr makeOpenClBufferManager(const OpenClBufferContainerArgs &);

//! Extract the cl_mem object from the managed buffer
cl_mem &getClBufferFromManaged(const Pothos::ManagedBuffer &buff);

/***********************************************************************
 * smart pointer deleters for managing cl objects
 **********************************************************************/
inline void clReleaseContextPtr(cl_context *p)
{
    clReleaseContext(*p);
}

inline void clReleaseProgramPtr(cl_program *p)
{
    clReleaseProgram(*p);
}

inline void clReleaseCommandQueuePtr(cl_command_queue *p)
{
    clReleaseCommandQueue(*p);
}

inline void clReleaseKernelPtr(cl_kernel *p)
{
    clReleaseKernel(*p);
}
