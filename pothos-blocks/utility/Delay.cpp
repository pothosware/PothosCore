// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memset

/***********************************************************************
 * |PothosDoc Delay
 *
 * The delay block imposes a constant delay in stream elements.
 * The implementation passively forwards inputs to outputs,
 * without incuring any memory copying overhead.
 *
 * |category /Utility
 * |keywords delay time
 *
 * |param delay The delay in number of stream elements.
 * |default 0
 *
 * |factory /blocks/delay(dtype)
 * |setter setDelay(delay)
 **********************************************************************/
class Delay : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new Delay();
    }

    Delay(void):
        _deltaElements(0),
        _actualDeltaElements(0)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(Delay, setDelay));
        this->registerCall(this, POTHOS_FCN_TUPLE(Delay, getDelay));
    }

    void setDelay(const int elements)
    {
        _deltaElements = elements;
    }

    int getDelay(void) const
    {
        return _deltaElements;
    }

    void work(void)
    {
        auto in0 = this->input(0);
        auto out0 = this->output(0);

        auto buffer = in0->buffer();
        if (buffer.length == 0) return; //dont act unless there is available input

        const auto delta = _actualDeltaElements - _deltaElements;

        //consume but not produce (drops elements)
        if (delta < 0)
        {
            const auto numElems = std::min(buffer.elements(), size_t(-delta));
            in0->consume(numElems*buffer.dtype.size());
            _actualDeltaElements += numElems;
            return;
        }

        //produce but not consume (inserts zeros)
        if (delta > 0)
        {
            Pothos::BufferChunk outBuff(buffer.dtype, size_t(delta));
            std::memset(outBuff.as<void *>(), 0, outBuff.length);
            out0->postBuffer(outBuff);
            _actualDeltaElements -= delta;
            return;
        }

        //otherwise just forward the buffer
        {
            out0->postBuffer(buffer);
            in0->consume(in0->elements());
        }
    }

private:
    int _deltaElements;
    int _actualDeltaElements;
};

static Pothos::BlockRegistry registerDelay(
    "/blocks/delay", &Delay::make);
