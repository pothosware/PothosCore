// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <iostream>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Freq Demod
 *
 * The frequency demodulation block consumes a complex input stream
 * on input port 0, performs a differential atan2 operation,
 * and outputs the real-valued changes in frequency
 * to the output stream on output port 0.
 *
 * |category /Demod
 * |keywords frequency modulation fm atan differential
 *
 * |param dtype[Data Type] The data type of the input stream.
 * |widget DTypeChooser(cfloat=1)
 * |default "complex_float32"
 * |preview disable
 *
 * |factory /comms/freq_demod(dtype)
 **********************************************************************/
template <typename Type>
class FreqDemod : public Pothos::Block
{
public:

    typedef typename Type::value_type OutType;

    FreqDemod(void)
    {
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(OutType));
    }

    void activate(void)
    {
        _prev = 0;
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        const size_t N = this->workInfo().minElements;

        //cast the input and output buffers
        const auto in = inPort->buffer().template as<const Type *>();
        const auto out = outPort->buffer().template as<OutType *>();

        for (size_t i = 0; i < N; i++)
        {
            auto in_i = in[i];
            auto diff = in_i * _prev;
            auto angle = std::atan2(diff.real(), diff.imag());
            _prev = std::conj(in_i);
            out[i] = OutType(angle);
        }

        inPort->consume(N);
        outPort->produce(N);
    }

private:
    Type _prev;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *FreqDemodFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(type))) return new FreqDemod<type>();
    ifTypeDeclareFactory(std::complex<double>);
    ifTypeDeclareFactory(std::complex<float>);
    throw Pothos::InvalidArgumentException("FreqDemodFactory("+dtype.toString()+")", "unsupported types");
}
static Pothos::BlockRegistry registerFreqDemod(
    "/comms/freq_demod", &FreqDemodFactory);
