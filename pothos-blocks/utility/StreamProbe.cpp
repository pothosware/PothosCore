// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Types.h>
#include <complex>
#include <iostream>

/***********************************************************************
 * |PothosDoc Stream Probe
 *
 * The stream probe block records the last calculation from a stream of elements.
 * The stream probe has a slot called "probeValue" will will cause
 * a signal named "valueTriggered" to emit the most recent value.
 *
 * The calculation for value can be, the last seen value,
 * the RMS (root mean square) over the last buffer,
 * or the mean (average value) over the last buffer.
 *
 * |category /Utility
 *
 * |param dtype[Data Type] The datatype consumed by the value probe.
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
 * |factory /blocks/stream_probe(dtype)
 * |setter setMode(mode)
 * |setter setWindow(window)
 **********************************************************************/
template <typename Type>
class StreamProbe : public Pothos::Block
{
public:
    StreamProbe(void):
        _value(0),
        _mode("VALUE"),
        _window(1024)
    {
        this->setupInput(0, typeid(Type));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamProbe, value));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamProbe, setMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamProbe, getMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamProbe, setWindow));
        this->registerCall(this, POTHOS_FCN_TUPLE(StreamProbe, getWindow));
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
        if (dtype == Pothos::DType(typeid(type))) return new StreamProbe<type>(); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) return new StreamProbe<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(Poco::Int64);
    ifTypeDeclareFactory(Poco::Int32);
    ifTypeDeclareFactory(Poco::Int16);
    ifTypeDeclareFactory(Poco::Int8);
    throw Pothos::InvalidArgumentException("valueProbeFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerStreamProbe(
    "/blocks/stream_probe", &valueProbeFactory);
