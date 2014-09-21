// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Types.h>
#include <complex>
#include <iostream>

template <typename T>
T rmsSqr(const std::complex<T> &val)
{
    return (val*std::conj(val)).real();
}

template <typename T>
T rmsSqr(const T &val)
{
    return val*val;
}

/***********************************************************************
 * |PothosDoc RMS Probe
 *
 * The RMS probe block records the last calculated RMS value from a stream of elements.
 * The RMS probe has a slot called "probeRms" will will cause
 * a signal named "rmsTriggered" to emit the most recent measurement.
 *
 * https://en.wikipedia.org/wiki/Root_mean_square
 *
 * |category /Math
 *
 * |param dtype[Data Type] The datatype consumed by the RMS probe.
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
 * |param window How many elements per RMS measurement?
 * |default 1024
 *
 * |factory /blocks/rms_probe(dtype)
 * |setter setWindow(window)
 **********************************************************************/
template <typename Type>
class RMSProbe : public Pothos::Block
{
public:
    RMSProbe(void):
        _rms(0),
        _window(1024)
    {
        this->setupInput(0, typeid(Type));
        this->registerCall(this, POTHOS_FCN_TUPLE(RMSProbe, rms));
        this->registerProbe("rms");
        this->setWindow(_window); //update reserve
    }

    Type rms(void)
    {
        return _rms;
    }

    void setWindow(const size_t window)
    {
        _window = window;
        this->input(0)->setReserve(window);
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto x = inPort->buffer().template as<const Type *>();
        const auto N = std::min(_window, inPort->elements());

        double accumulator = 0.0;
        for (size_t n = 0; n < N; n++)
        {
            accumulator += rmsSqr(x[n]);
        }
        _rms = Type(std::sqrt(accumulator/N));

        inPort->consume(N);
    }

private:
    Type _rms;
    size_t _window;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *rmsProbeFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) return new RMSProbe<type>(); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) return new RMSProbe<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(Poco::Int64);
    ifTypeDeclareFactory(Poco::Int32);
    ifTypeDeclareFactory(Poco::Int16);
    ifTypeDeclareFactory(Poco::Int8);
    throw Pothos::InvalidArgumentException("rmsProbeFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerRMSProbe(
    "/blocks/rms_probe", &rmsProbeFactory);
