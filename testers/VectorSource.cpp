// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <vector>
#include <complex>
#include <cstring> //memcpy
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Vector Source
 *
 * The vector source block creates a stream of elements from a user-specified list.
 * The primary use of this block is to test and debug other blocks.
 *
 * |category /Testers
 * |category /Sources
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
 * |param startId[Start ID] The label ID to mark the first element of the vector.
 * An empty string (default) means that start of vector labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 * |tab Labels
 *
 * |param endId[End ID] The label ID to mark the last element of the vector.
 * An empty string (default) means that end of vector labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 * |tab Labels
 *
 * |factory /blocks/vector_source(dtype)
 * |setter setMode(mode)
 * |setter setElements(elements)
 * |setter setStartId(startId)
 * |setter setEndId(endId)
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
        this->registerCall(this, POTHOS_FCN_TUPLE(VectorSource, setStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(VectorSource, setEndId));
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
        _pending = Pothos::BufferChunk();
    }

    void setMode(const std::string &mode)
    {
        if (mode == "REPEAT") _repeat = true;
        else if (mode == "ONCE") _repeat = false;
        else throw Pothos::InvalidArgumentException("VectorSource::setMode("+mode+")", "unknown mode");
    }

    void setStartId(const std::string &id)
    {
        _startId = id;
    }

    void setEndId(const std::string &id)
    {
        _endId = id;
    }

    void activate(void)
    {
        _once = false;
        _pending = Pothos::BufferChunk();
    }

    void work(void)
    {
        //not repeat mode and we already did work once:
        if (not _repeat and _once) return;

        auto outPort = this->output(0);
        auto outBuff = outPort->buffer();

        //begin the pending buffer again
        if (_pending.length == 0)
        {
            _pending = _elems;
            if (not _startId.empty()) outPort->postLabel(
                Pothos::Label(_startId, _elems.elements(), 0));
        }

        //copy into the output buffer
        const auto numElems = std::min(_pending.elements(), outPort->elements());
        const auto numBytes = numElems*outPort->dtype().size();
        std::memcpy(outBuff.as<void *>(), _pending.as<const void *>(), numBytes);
        outPort->produce(numElems);

        //consume from the pending buffer
        _pending.address += numBytes;
        _pending.length -= numBytes;

        //completed the pending buffer
        if (_pending.length == 0)
        {
            _once = true;
            if (not _endId.empty()) outPort->postLabel(
                Pothos::Label(_endId, _elems.elements(), numElems-1));
        }
    }

private:
    Pothos::BufferChunk _elems;
    Pothos::BufferChunk _pending;
    bool _repeat;
    bool _once;
    std::string _startId;
    std::string _endId;
};

static Pothos::BlockRegistry registerVectorSource(
    "/blocks/vector_source", &VectorSource::make);
