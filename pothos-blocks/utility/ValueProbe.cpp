// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Types.h>
#include <complex>
#include <iostream>

/***********************************************************************
 * |PothosDoc Value Probe
 *
 * The value probe block records the last seen value from a stream of elements.
 * The value probe has a slot called "probeValue" will will cause
 * a signal named "valueTriggered" to emit the most recent value.
 *
 * This block is intented to be fed by an upstream block that produces
 * a stream of slow-changing values such as a moving-average or RMS.
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
 * |factory /blocks/value_probe(dtype)
 **********************************************************************/
template <typename Type>
class ValueProbe : public Pothos::Block
{
public:
    ValueProbe(void)
    {
        this->setupInput(0, typeid(Type));
        this->registerCall(this, POTHOS_FCN_TUPLE(ValueProbe, value));
        this->registerProbe("value");
        this->input(0)->setReserve(1);
    }

    Type value(void)
    {
        return _value;
    }

    void work(void)
    {
        //store most recent value and consume entire buffer
        auto inputPort = this->input(0);
        const auto elems = inputPort->buffer().template as<const Type *>();
        _value = elems[inputPort->elements()-1];
        inputPort->consume(inputPort->elements());
    }

private:
    Type _value;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *valueProbeFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) return new ValueProbe<type>(); \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) return new ValueProbe<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(Poco::Int64);
    ifTypeDeclareFactory(Poco::Int32);
    ifTypeDeclareFactory(Poco::Int16);
    ifTypeDeclareFactory(Poco::Int8);
    throw Pothos::InvalidArgumentException("valueProbeFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerValueProbe(
    "/blocks/value_probe", &valueProbeFactory);
