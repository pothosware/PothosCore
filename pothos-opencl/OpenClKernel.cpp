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
class OpenClKernel : public Pothos::Block
{
public:
    static Pothos::Block *make(void)
    {
        return new OpenClKernel();
    }

    OpenClKernel(void):
        _localSize(1),
        _globalFactor(1.0),
        _productionFactor(1.0)
    {
        this->registerCall(POTHOS_FCN_TUPLE(OpenClKernel, setSource));
        this->registerCall(POTHOS_FCN_TUPLE(OpenClKernel, setLocalSize));
        this->registerCall(POTHOS_FCN_TUPLE(OpenClKernel, getLocalSize));
        this->registerCall(POTHOS_FCN_TUPLE(OpenClKernel, setGlobalFactor));
        this->registerCall(POTHOS_FCN_TUPLE(OpenClKernel, getGlobalFactor));
        this->registerCall(POTHOS_FCN_TUPLE(OpenClKernel, setProductionFactor));
        this->registerCall(POTHOS_FCN_TUPLE(OpenClKernel, getProductionFactor));
    }

    void setSource(const std::string &name, const std::string &source)
    {
        _kernelName = name;
        _kernelSource = source;
    }

    void setLocalSize(const size_t size)
    {
        _localSize = size;
    }

    size_t getLocalSize(void) const
    {
        return _localSize;
    }

    void setGlobalFactor(const double factor)
    {
        _globalFactor = factor;
    }

    double getGlobalFactor(void) const
    {
        return _globalFactor;
    }

    void setProductionFactor(const double factor)
    {
        _productionFactor = factor;
    }

    double getProductionFactor(void) const
    {
        return _productionFactor;
    }

    void activate(void);
    void deactivate(void);
    void work(void);

private:
    cl_device_id _device;
    cl_context _context;
    cl_program _program;
    cl_kernel _kernel;
    cl_command_queue _queue;
    std::string _kernelName;
    std::string _kernelSource;
    size_t _localSize;
    double _globalFactor;
    double _productionFactor;
};

void OpenClKernel::activate(void)
{
    cl_platform_id platform;
    int err = 0;

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, nullptr);
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::identifyPlatform()", std::to_string(err));

    /* Access a device */
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &_device, nullptr);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &_device, nullptr);
    }
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::accessDevice()", std::to_string(err));

    /* Create context */
    _context = clCreateContext(nullptr, 1, &_device, nullptr, nullptr, &err);
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::createContext()", std::to_string(err));

    /* Create program from source */
    if (_kernelSource.empty()) throw Pothos::Exception("OpenClKernel::activate::createProgram()", "no source specified");
    const char *sourcePtr = _kernelSource.data();
    const size_t sourceSize = _kernelSource.size();
    _program = clCreateProgramWithSource(_context, 1, &sourcePtr, &sourceSize, &err);
    if(err < 0) throw Pothos::Exception("OpenClKernel::activate::createProgram()", std::to_string(err));

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
        throw Pothos::Exception("OpenClKernel::activate::buildProgram()", errorString);
    }

    /* Create a command queue */
    _queue = clCreateCommandQueue(_context, _device, 0, &err);
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::createCommandQueue()", std::to_string(err));

    /* Create a kernel */
    _kernel = clCreateKernel(_program, _kernelName.c_str(), &err);
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::createKernel()", std::to_string(err));
}

void OpenClKernel::deactivate(void)
{
    clReleaseKernel(_kernel);
    clReleaseCommandQueue(_queue);
    clReleaseProgram(_program);
    clReleaseContext(_context);
}

void OpenClKernel::work(void)
{
    const auto &inputs = this->inputs();
    const auto &outputs = this->outputs();

    std::vector<cl_mem> inputBuffs(inputs.size());
    std::vector<cl_mem> outputBuffs(outputs.size());

    int err = 0;

    if (this->workInfo().minElements == 0) return;

    //calculate number of elements
    size_t inputElems = this->workInfo().minInElements;
    size_t outputElems = this->workInfo().minOutElements;
    if (_productionFactor > 1.0)
    {
        outputElems = std::min<size_t>(inputElems*_productionFactor, outputElems);
        inputElems = outputElems/_productionFactor;
    }
    else
    {
        inputElems = std::min<size_t>(outputElems/_productionFactor, inputElems);
        outputElems = inputElems*_productionFactor;
    }
    size_t globalSize = inputElems*_globalFactor;

    /* Create data buffer */
    size_t argNo = 0;
    for (size_t i = 0; i < inputs.size(); i++)
    {
        inputBuffs[i] = clCreateBuffer(_context, CL_MEM_READ_ONLY |
         CL_MEM_COPY_HOST_PTR, inputElems*inputs[i]->dtype().size(), inputs[i]->buffer().as<void *>(), &err);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clCreateBuffer()", std::to_string(err));
        err = clSetKernelArg(_kernel, argNo++, sizeof(cl_mem), &inputBuffs[i]);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clSetKernelArg()", std::to_string(err));
    }
    for (size_t i = 0; i < outputs.size(); i++)
    {
        outputBuffs[i] = clCreateBuffer(_context, CL_MEM_READ_WRITE |
         CL_MEM_COPY_HOST_PTR, outputElems*outputs[i]->dtype().size(), outputs[i]->buffer().as<void *>(), &err);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clCreateBuffer()", std::to_string(err));
        err = clSetKernelArg(_kernel, argNo++, sizeof(cl_mem), &outputBuffs[i]);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clSetKernelArg()", std::to_string(err));
    }

    /* Enqueue kernel */
    err = clEnqueueNDRangeKernel(_queue, _kernel, 1, nullptr, &globalSize, &_localSize, 0, nullptr, nullptr); 
    if (err < 0) throw Pothos::Exception("OpenClKernel::work::enqueueKernel()", std::to_string(err));

    /* Read the kernel's output */
    for (size_t i = 0; i < inputs.size(); i++)
    {
        inputs[i]->consume(inputElems);
        clReleaseMemObject(inputBuffs[i]);
    }
    for (size_t i = 0; i < outputs.size(); i++)
    {
        err = clEnqueueReadBuffer(_queue, outputBuffs[i], CL_TRUE, 0, 
            outputElems*outputs[i]->dtype().size(), outputs[i]->buffer().as<void *>(), 0, nullptr, nullptr);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clEnqueueReadBuffer()", std::to_string(err));
        outputs[i]->produce(outputElems);
        clReleaseMemObject(outputBuffs[i]);
    }
}

static Pothos::BlockRegistry registerOpenClKernel(
    "/blocks/opencl/opencl_kernel", &OpenClKernel::make);
