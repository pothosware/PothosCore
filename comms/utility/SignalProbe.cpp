// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <iostream>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Signal Probe
 *
 * The signal probe block records the last calculation from a stream of elements.
 * The signal probe has a slot called "probeValue" will will cause
 * a signal named "valueTriggered" to emit the most recent value.
 *
 * The calculation for value can be, the last seen value,
 * the RMS (root mean square) over the last buffer,
 * or the mean (average value) over the last buffer.
 *
 * |category /Utility
 * |category /Event
 * |keywords rms average mean
 * |alias /blocks/stream_probe
 *
 * |param dtype[Data Type] The data type consumed by the stream probe.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param mode The calculation mode for the value.
 * In value mode, this block expects to be fed by an upstream block
 * that produces a stream of slow-changing values.
 * Otherwise the value will appear random.
 * |default "VALUE"
 * |option [Value] "VALUE"
 * |option [RMS] "RMS"
 * |option [Mean] "MEAN"
 *
 * |param window How many elements to calculate over?
 * |default 1024
 *
 * |factory /comms/signal_probe(dtype)
 * |setter setMode(mode)
 * |setter setWindow(window)
 **********************************************************************/
template <typename Type>
class SignalProbe : public Pothos::Block
{
public:
    SignalProbe(void):
        _value(0),
        _mode("VALUE"),
        _window(1024)
    {
        this->setupInput(0, typeid(Type));
        this->registerCall(this, POTHOS_FCN_TUPLE(SignalProbe, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(SignalProbe, setMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(SignalProbe, getMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(SignalProbe, setWindow));
        this->registerCall(this, POTHOS_FCN_TUPLE(SignalProbe, getWindow));
        this->registerProbe("value");
        this->input(0)->setReserve(1);
    }

    Type value(void)
    {
        return _value;
    }

    void setMode(const std::string &mode)
    {
        _mode = mode;
    }

    std::string getMode(void) const
    {
        return _mode;
    }

    void setWindow(const size_t window)
    {
        _window = window;
        this->input(0)->setReserve(window);
    }

    size_t getWindow(void) const
    {
        return _window;
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto x = inPort->buffer().template as<const Type *>();
        const auto N = std::min(_window, inPort->elements());
        inPort->consume(N);

        if (_mode == "VALUE") _value = x[N-1];
        else if (_mode == "RMS")
        {
            Type mean = 0;
            for (size_t n = 0; n < N; n++) mean += x[n];
            mean /= N;

            double accumulator = 0.0;
            for (size_t n = 0; n < N; n++)
            {
                const auto v = std::abs(x[n]-mean);
                accumulator += v*v;
            }
            _value = Type(std::sqrt(accumulator/N));
        }
        else if (_mode == "MEAN")
        {
            Type mean = 0;
            for (size_t n = 0; n < N; n++) mean += x[n];
            mean /= N;
            _value = std::abs(mean);
        }
    }

private:
    Type _value;
    std::string _mode;
    size_t _window;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *valueProbeFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) return new SignalProbe<type>(); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) return new SignalProbe<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("valueProbeFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerSignalProbe(
    "/comms/signal_probe", &valueProbeFactory);

static Pothos::BlockRegistry registerSignalProbeOldPath(
    "/blocks/stream_probe", &valueProbeFactory);
