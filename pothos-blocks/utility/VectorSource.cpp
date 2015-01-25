// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <vector>
#include <complex>
#include <cstring> //memcpy
#include <thread>

/***********************************************************************
 * |PothosDoc Vector Source
 *
 * The vector source block creates a stream of elements from a user-specified list.
 * The primary use of this block is to test and debug other blocks.
 *
 * |category /Utility
 * |keywords test
 *
 * |param dtype[Data Type] The output data type.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param mode The work operation mode for the vector source.
 * The source can either repeat the elements back to back - indefinitely,
 * or stop producing until the elements are changed or the block is activated again.
 * |option [Repeat] "REPEAT"
 * |option [Once] "ONCE"
 * |default "REPEAT"
 *
 * |param elements Specify a list of elements to produce.
 * |default [1.0, 2.0, 3.0]
 *
 * |factory /blocks/vector_source(dtype)
 * |setter setMode(mode)
 * |setter setElements(elements)
 **********************************************************************/
class VectorSource : public Pothos::Block
{
public:
    static Block *make(const Pothos::DType &dtype)
    {
        return new VectorSource(dtype);
    }

    VectorSource(const Pothos::DType &dtype):
        _repeat(false),
        _once(false)
    {
        this->setupOutput(0, dtype);
        this->registerCall(this, POTHOS_FCN_TUPLE(VectorSource, setElements));
        this->registerCall(this, POTHOS_FCN_TUPLE(VectorSource, setMode));
    }

    void setElements(const std::vector<std::complex<double>> &elems)
    {
        //copy this vector into a buffer chunk
        Pothos::BufferChunk newElems(Pothos::DType(typeid(std::complex<double>)), elems.size());
        std::memcpy(newElems.as<void *>(), elems.data(), elems.size()*sizeof(std::complex<double>));

        //use buffer chunk convert to store in output port format
        if (this->output(0)->dtype().isComplex())
        {
            _elems = newElems.convert(this->output(0)->dtype());
        }
        //when its not complex, just take the real part
        else
        {
            _elems = newElems.convertComplex(this->output(0)->dtype()).first;
        }

        //reset flag so we can output again
        _once = false;
    }

    void setMode(const std::string &mode)
    {
        if (mode == "REPEAT") _repeat = true;
        else if (mode == "ONCE") _repeat = false;
        else throw Pothos::Exception("VectorSource::setMode("+mode+")", "unknown mode");
    }

    void activate(void)
    {
        _once = false;
    }

    void work(void)
    {
        //not repeat mode and we already did work once?
        //just wait the maximum timeout so we dont steal the CPU
        if (not _repeat and _once)
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(this->workInfo().maxTimeoutNs));
            return this->yield();
        }

        auto outPort = this->output(0);
        auto outBuff = outPort->buffer();

        //copy in the user-specified elements
        //The reason we simply don't just post the entire buffer
        //is to use the output buffer resources as back-pressure,
        //in case the user want to use this block in a live stream.
        const auto numElems = std::min(_elems.elements(), outPort->elements());
        outBuff.length = numElems*outPort->dtype().size();
        std::memcpy(outBuff.as<void *>(), _elems.as<const void *>(), outBuff.length);

        //rather than produce, we pop and post because of the additional optimization below
        {
            outPort->popBuffer(outBuff.length);
            outPort->postBuffer(outBuff);
        }

        //was the user-specified elements so large that we couldn't copy it all?
        //in this case just post the remainder of the internal buffer.
        if (numElems < _elems.elements())
        {
            auto subBuff = _elems;
            const auto bytesOff = (_elems.elements() - numElems)*outPort->dtype().size();
            subBuff.address += bytesOff;
            subBuff.length -= bytesOff;
            outPort->postBuffer(subBuff);
        }

        _once = true;
    }

private:
    Pothos::BufferChunk _elems;
    bool _repeat;
    bool _once;
};

static Pothos::BlockRegistry registerVectorSource(
    "/blocks/vector_source", &VectorSource::make);
