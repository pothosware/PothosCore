// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc Finite Release
 *
 * The finite release block passivly forwards all data from
 * input port 0 to the output port 0 without copying until
 * a specified count of total elements is reached.
 * This block is mainly used for unit testing purposes.
 *
 * When the count is reached, the block back-pressures the input.
 * The count resets and the flow resumes when the block is re-activated
 * or when the total element count is reset.
 *
 * |category /Utility
 *
 * |param dtype[Data Type] The datatype of the element stream.
 * |preview disable
 * |option [Complex128] "complex128"
 * |option [Float64] "float64"
 * |option [Complex64] "complex64"
 * |option [Float32] "float32"
 * |option [Complex Int64] "complex_int64"
 * |option [Int64] "int64"
 * |option [Complex Int32] "complex_int32"
 * |option [Int32] "int32"
 * |option [Complex Int16] "complex_int16"
 * |option [Int16] "int16"
 * |option [Complex Int8] "complex_int8"
 * |option [Int8] "int8"
 * |widget ComboBox(editable=true)
 *
 * |param total[Total Elements] The total number of elements to allow through the block.
 * |default 1024
 * |widget SpinBox(minimum=0)
 *
 * |factory /blocks/finite_release(dtype)
 * |setter setTotalElements(total)
 **********************************************************************/
class FiniteRelease : public Pothos::Block
{
public:
    static Block *make(const Pothos::DType &dtype)
    {
        return new FiniteRelease(dtype);
    }

    FiniteRelease(const Pothos::DType &dtype):
        _totalElements(1024),
        _elementsLeft(1024)
    {
        this->setupInput(0, dtype);
        this->setupOutput(0, dtype, this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(FiniteRelease, setTotalElements));
        this->registerCall(this, POTHOS_FCN_TUPLE(FiniteRelease, getTotalElements));
        this->registerCall(this, POTHOS_FCN_TUPLE(FiniteRelease, getElementsLeft));
    }


    void activate(void)
    {
        //restart the count
        _elementsLeft = _totalElements;
    }

    void setTotalElements(const size_t total)
    {
        _totalElements = total;
        _elementsLeft = _totalElements;
    }

    size_t getTotalElements(void) const
    {
        return _totalElements;
    }

    size_t getElementsLeft(void) const
    {
        return _elementsLeft;
    }

    void work(void)
    {
        auto inputPort = this->input(0);
        auto outputPort = this->output(0);

        while (inputPort->hasMessage() and _elementsLeft != 0)
        {
            auto m = inputPort->popMessage();
            outputPort->postMessage(m);
            _elementsLeft -= 1;
        }

        const auto &buffer = inputPort->buffer();
        const size_t elems = std::min(_elementsLeft, inputPort->elements());
        if (elems != 0)
        {
            outputPort->postBuffer(buffer);
            inputPort->consume(elems);
            _elementsLeft -= elems;
        }
    }

private:
    size_t _totalElements;
    size_t _elementsLeft;
};

static Pothos::BlockRegistry registerFiniteRelease(
    "/blocks/finite_release", &FiniteRelease::make);
