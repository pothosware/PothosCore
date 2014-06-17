// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "OpenClBufferManager.hpp"
#include <Pothos/Framework.hpp>
#include <iostream>

/***********************************************************************
 * TODO
 * enumerate opencl devices
 * pick an idenfier scheme for the constructor
 * set a domain on each input/output port
 * look up table for device number to context
 * topology needs to collect domain info and
 *  - call an overload on the block to create a manager
 *  - create copier blocks to allow domains to interact
 **********************************************************************/
class OpenClKernel : public Pothos::Block
{
public:
    static Pothos::Block *make(const size_t platformIndex, const size_t deviceIndex)
    {
        return new OpenClKernel(platformIndex, deviceIndex);
    }

    OpenClKernel(const size_t platformIndex, const size_t deviceIndex):
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

    ~OpenClKernel(void)
    {
        return;
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
    std::shared_ptr<cl_context> _context;
    std::shared_ptr<cl_program> _program;
    std::shared_ptr<cl_kernel> _kernel;
    std::shared_ptr<cl_command_queue> _queue;
    std::string _kernelName;
    std::string _kernelSource;
    size_t _localSize;
    double _globalFactor;
    double _productionFactor;
};

void clReleaseContextPtr(cl_context *p)
{
    clReleaseContext(*p);
}

void clReleaseProgramPtr(cl_program *p)
{
    clReleaseProgram(*p);
}

void clReleaseCommandQueuePtr(cl_command_queue *p)
{
    clReleaseCommandQueue(*p);
}

void clReleaseKernelPtr(cl_kernel *p)
{
    clReleaseKernel(*p);
}

void OpenClKernel::activate(void)
{
    cl_platform_id platform;
    cl_int err = 0;

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, nullptr);
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::clGetPlatformIDs()", std::to_string(err));

    /* Access a device */
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &_device, nullptr);
    if (err == CL_DEVICE_NOT_FOUND)
    {
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &_device, nullptr);
    }
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::clGetDeviceIDs()", std::to_string(err));

    /* Create context */
    auto context = clCreateContext(nullptr, 1, &_device, nullptr, nullptr, &err);
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::clCreateContext()", std::to_string(err));
    _context.reset(new cl_context(context), &clReleaseContextPtr);

    /* Create program from source */
    if (_kernelSource.empty()) throw Pothos::Exception("OpenClKernel::activate::createProgram()", "no source specified");
    const char *sourcePtr = _kernelSource.data();
    const size_t sourceSize = _kernelSource.size();
    auto program = clCreateProgramWithSource(*_context, 1, &sourcePtr, &sourceSize, &err);
    if(err < 0) throw Pothos::Exception("OpenClKernel::activate::clCreateProgramWithSource()", std::to_string(err));
    _program.reset(new cl_program(program), clReleaseProgramPtr);

    /* Build program */
    err = clBuildProgram(*_program, 0, nullptr, nullptr, nullptr, nullptr);
    if (err < 0)
    {
        /* Find size of log and print to std output */
        size_t logSize = 0;
        clGetProgramBuildInfo(*_program, _device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> errorLog(logSize);
        clGetProgramBuildInfo(*_program, _device, CL_PROGRAM_BUILD_LOG, logSize, errorLog.data(), nullptr);

        std::string errorString(errorLog.begin(), errorLog.end());
        throw Pothos::Exception("OpenClKernel::activate::clBuildProgram()", errorString);
    }

    /* Create a command queue */
    auto queue = clCreateCommandQueue(*_context, _device, 0, &err);
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::clCreateCommandQueue()", std::to_string(err));
    _queue.reset(new cl_command_queue(queue), clReleaseCommandQueuePtr);

    /* Create a kernel */
    auto kernel = clCreateKernel(*_program, _kernelName.c_str(), &err);
    if (err < 0) throw Pothos::Exception("OpenClKernel::activate::clCreateKernel()", std::to_string(err));
    _kernel.reset(new cl_kernel(kernel), clReleaseKernelPtr);

    /* Set the output buffer managers */
    const auto &outputs = this->outputs();
    for (size_t i = 0; i < outputs.size(); i++)
    {
        OpenClBufferContainerArgs args;
        args.mem_flags = CL_MEM_WRITE_ONLY;
        args.map_flags = CL_MAP_READ;
        args.context = _context;
        args.queue = _queue;
        auto manager = makeOpenClBufferManager(args);
        //manager->init(Pothos::BufferManagerArgs());
        //outputs[i]->setBufferManager(manager);
    }
}

void OpenClKernel::deactivate(void)
{
    //reset in order of creation
    _kernel.reset();
    _queue.reset();
    _program.reset();
    _context.reset();
}

void OpenClKernel::work(void)
{
    const auto &inputs = this->inputs();
    const auto &outputs = this->outputs();

    std::vector<cl_mem> inputBuffs(inputs.size());
    std::vector<cl_mem> outputBuffs(outputs.size());

    cl_int err = 0;

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
        inputBuffs[i] = clCreateBuffer(*_context, CL_MEM_READ_ONLY |
         CL_MEM_COPY_HOST_PTR, inputElems*inputs[i]->dtype().size(), inputs[i]->buffer().as<void *>(), &err);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clCreateBuffer()", std::to_string(err));
        err = clSetKernelArg(*_kernel, argNo++, sizeof(cl_mem), &inputBuffs[i]);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clSetKernelArg()", std::to_string(err));
    }
    for (size_t i = 0; i < outputs.size(); i++)
    {
        //outputBuffs[i] = clCreateBuffer(*_context, CL_MEM_READ_WRITE |
        // CL_MEM_COPY_HOST_PTR, outputElems*outputs[i]->dtype().size(), outputs[i]->buffer().as<void *>(), &err);
        outputBuffs[i] = getClBufferFromManaged(outputs[i]->buffer().getManagedBuffer());
        //if (err < 0) throw Pothos::Exception("OpenClKernel::work::clCreateBuffer()", std::to_string(err));
        err = clSetKernelArg(*_kernel, argNo++, sizeof(cl_mem), &outputBuffs[i]);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clSetKernelArg()", std::to_string(err));
    }

    /* Enqueue kernel */
    err = clEnqueueNDRangeKernel(*_queue, *_kernel, 1, nullptr, &globalSize, &_localSize, 0, nullptr, nullptr);
    if (err < 0) throw Pothos::Exception("OpenClKernel::work::enqueueKernel()", std::to_string(err));

    /* Read the kernel's output */
    for (size_t i = 0; i < inputs.size(); i++)
    {
        inputs[i]->consume(inputElems);
        clReleaseMemObject(inputBuffs[i]);
    }
    for (size_t i = 0; i < outputs.size(); i++)
    {
        err = clEnqueueReadBuffer(*_queue, outputBuffs[i], CL_TRUE, 0,
            outputElems*outputs[i]->dtype().size(), outputs[i]->buffer().as<void *>(), 0, nullptr, nullptr);
        if (err < 0) throw Pothos::Exception("OpenClKernel::work::clEnqueueReadBuffer()", std::to_string(err));
        outputs[i]->produce(outputElems);
        //clReleaseMemObject(outputBuffs[i]);
    }
}

static Pothos::BlockRegistry registerOpenClKernel(
    "/blocks/opencl/opencl_kernel", &OpenClKernel::make);
