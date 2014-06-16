// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <iostream>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

/***********************************************************************
 * 
 **********************************************************************/
class OpenClBlock : public Pothos::Block
{
public:
    static Pothos::Block *make(void)
    {
        return new OpenClBlock();
    }

    OpenClBlock(void)
    {
        this->setupInput("0");
        this->setupOutput("0");
        this->registerCall(POTHOS_FCN_TUPLE(OpenClBlock, setSource));
    }

    void setSource(const std::string &name, const std::string &source)
    {
        _kernelName = name;
        _kernelSource = source;
    }

    void activate(void);

    void work(void);

private:
    cl_device_id _device;
    cl_context _context;
    cl_program _program;
    cl_kernel _kernel;
    cl_command_queue _queue;
    std::string _kernelName;
    std::string _kernelSource;
};

void OpenClBlock::activate(void)
{
    cl_platform_id platform;
    int err = 0;

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, nullptr);
    if (err < 0) throw Pothos::Exception("OpenClBlock::activate::identifyPlatform()", std::to_string(err));

    /* Access a device */
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &_device, nullptr);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &_device, nullptr);
    }
    if (err < 0) throw Pothos::Exception("OpenClBlock::activate::accessDevice()", std::to_string(err));

    /* Create context */
    _context = clCreateContext(nullptr, 1, &_device, nullptr, nullptr, &err);
    if (err < 0) throw Pothos::Exception("OpenClBlock::activate::createContext()", std::to_string(err));

    /* Create program from source */
    if (_kernelSource.empty()) throw Pothos::Exception("OpenClBlock::activate::createProgram()", "no source specified");
    const char *sourcePtr = _kernelSource.data();
    const size_t sourceSize = _kernelSource.size();
    _program = clCreateProgramWithSource(_context, 1, &sourcePtr, &sourceSize, &err);
    if(err < 0) throw Pothos::Exception("OpenClBlock::activate::createProgram()", std::to_string(err));

    /* Build program */
    err = clBuildProgram(_program, 0, nullptr, nullptr, nullptr, nullptr);
    if (err < 0)
    {
        /* Find size of log and print to std output */
        size_t logSize = 0;
        clGetProgramBuildInfo(_program, _device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> errorLog(logSize);
        clGetProgramBuildInfo(_program, _device, CL_PROGRAM_BUILD_LOG, logSize, errorLog.data(), nullptr);

        std::string errorString(errorLog.begin(), errorLog.end());
        throw Pothos::Exception("OpenClBlock::activate::buildProgram()", errorString);
    }

    /* Create a command queue */
    _queue = clCreateCommandQueue(_context, _device, 0, &err);
    if (err < 0) throw Pothos::Exception("OpenClBlock::activate::createCommandQueue()", std::to_string(err));

    /* Create a kernel */
    _kernel = clCreateKernel(_program, _kernelName.c_str(), &err);
    if (err < 0) throw Pothos::Exception("OpenClBlock::activate::createKernel()", std::to_string(err));
}

void OpenClBlock::work(void)
{
    const auto &inputs = this->inputs();
    const auto &outputs = this->outputs();

    std::vector<cl_mem> inputBuffs(inputs.size());
    std::vector<cl_mem> outputBuffs(outputs.size());

    int err = 0;
    size_t elems = this->workInfo().minElements;
    if (elems == 0) return;

    size_t global_size = elems;
    size_t local_size = 1;

    /* Create data buffer */
    size_t argNo = 0;
    for (size_t i = 0; i < inputs.size(); i++)
    {
        inputBuffs[i] = clCreateBuffer(_context, CL_MEM_READ_ONLY |
         CL_MEM_COPY_HOST_PTR, inputs[i]->buffer().length, inputs[i]->buffer().as<void *>(), &err);
        if (err < 0) throw Pothos::Exception("OpenClBlock::work::clCreateBuffer()", std::to_string(err));
        err = clSetKernelArg(_kernel, argNo++, sizeof(cl_mem), &inputBuffs[i]);
        if (err < 0) throw Pothos::Exception("OpenClBlock::work::clSetKernelArg()", std::to_string(err));
    }
    for (size_t i = 0; i < outputs.size(); i++)
    {
        outputBuffs[i] = clCreateBuffer(_context, CL_MEM_READ_WRITE |
         CL_MEM_COPY_HOST_PTR, outputs[i]->buffer().length, outputs[i]->buffer().as<void *>(), &err);
        if (err < 0) throw Pothos::Exception("OpenClBlock::work::clCreateBuffer()", std::to_string(err));
        err = clSetKernelArg(_kernel, argNo++, sizeof(cl_mem), &outputBuffs[i]);
        if (err < 0) throw Pothos::Exception("OpenClBlock::work::clSetKernelArg()", std::to_string(err));
    }

    /* Enqueue kernel */
    err = clEnqueueNDRangeKernel(_queue, _kernel, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr); 
    if (err < 0) throw Pothos::Exception("OpenClBlock::work::enqueueKernel()", std::to_string(err));

    /* Read the kernel's output */
    for (size_t i = 0; i < inputs.size(); i++)
    {
        inputs[i]->consume(elems);
    }
    for (size_t i = 0; i < outputs.size(); i++)
    {
        err = clEnqueueReadBuffer(_queue, outputBuffs[i], CL_TRUE, 0, 
            elems*1, outputs[i]->buffer().as<void *>(), 0, nullptr, nullptr);
        if (err < 0) throw Pothos::Exception("OpenClBlock::work::clEnqueueReadBuffer()", std::to_string(err));
        outputs[i]->produce(elems);
    }
}

static Pothos::BlockRegistry registerOpenClBlock(
    "/blocks/opencl/opencl_block", &OpenClBlock::make);
