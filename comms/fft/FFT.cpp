// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <cmath>
//#include "kissfft.hh"

/***********************************************************************
 * |PothosDoc FFT
 *
 * TODO
 *
 * |category /FFT
 * |keywords dft fft fast fourier transform
 *
 * |param dtype[Data Type] The data type of the input and output element stream.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |factory /comms/fft(dtype)
 **********************************************************************/
template <typename Type>
class FFT : public Pothos::Block
{
public:
    FFT(void)//:
    //    _kissfft(1024, false)
    {
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type));
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);
    }

private:
    //kissfft<typename Type::value_type> _kissfft;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *FFTFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory__(Type) \
        if (dtype == Pothos::DType(typeid(Type))) return new FFT<Type>();
    #define ifTypeDeclareFactory(Type) \
        ifTypeDeclareFactory__(std::complex<Type>)
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    //ifTypeDeclareFactory(int64_t);
    //ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    //ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("FFTFactory("+dtype.toString()+")", "unsupported type");
}
static Pothos::BlockRegistry registerFFT(
    "/comms/fft", &FFTFactory);
