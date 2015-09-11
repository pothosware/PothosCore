// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <cmath>
#include "kissfft.hh"
#include "kiss_fft.h"

/***********************************************************************
 * |PothosDoc FFT
 *
 * Perform a Fast Fourier Transform on input port 0
 * and produce the FFT result to output port 0.
 *
 * |category /FFT
 * |keywords dft fft fast fourier transform
 *
 * |param dtype[Data Type] The data type of the input and output element stream.
 * |widget DTypeChooser(cfloat=1, cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param numBins[Num FFT Bins] The number of bins per fourier transform.
 * |default 1024
 * |option 512
 * |option 1024
 * |option 2048
 * |option 4096
 * |widget ComboBox(editable=true)
 *
 * |param inverse[Inverse FFT] The option to perform the inverse for forward FFT.
 * |option [Forward] false
 * |option [Inverse] true
 * |default false
 *
 * |factory /comms/fft(dtype, numBins, inverse)
 **********************************************************************/
template <typename Type>
class FFT : public Pothos::Block
{
public:
    FFT(const size_t numBins, const bool inverse):
        _numBins(numBins),
        _inverse(inverse),
        _fftFloat64(nullptr),
        _fftFloat32(nullptr),
        _fftFixedXX(nullptr)
    {
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type));
        this->input(0)->setReserve(_numBins);

        if (std::is_same<typename Type::value_type, double>::value)
        {
            _fftFloat64 = new kissfft<double>(numBins, inverse);
        }

        if (std::is_same<typename Type::value_type, float>::value)
        {
            _fftFloat32 = new kissfft<float>(numBins, inverse);
        }

        if (std::is_same<typename Type::value_type, kiss_fft_scalar>::value)
        {
            _fftFixedXX = kiss_fft_alloc(numBins, inverse, nullptr, nullptr);
        }
    }

    ~FFT(void)
    {
        delete _fftFloat64;
        delete _fftFloat32;
        kiss_fft_free(_fftFixedXX);
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        if (_fftFloat64 != nullptr) _fftFloat64->transform(
            inPort->buffer().template as<const std::complex<double> *>(),
            outPort->buffer().template as<std::complex<double> *>());

        else if (_fftFloat32 != nullptr) _fftFloat32->transform(
            inPort->buffer().template as<const std::complex<float> *>(),
            outPort->buffer().template as<std::complex<float> *>());

        else if (_fftFixedXX != nullptr) kiss_fft(_fftFixedXX,
            inPort->buffer().template as<const kiss_fft_cpx *>(),
            outPort->buffer().template as<kiss_fft_cpx *>());

        inPort->consume(_numBins);
        outPort->produce(_numBins);
    }

private:
    const size_t _numBins;
    const bool _inverse;
    kissfft<double> *_fftFloat64;
    kissfft<float> *_fftFloat32;
    kiss_fft_cfg _fftFixedXX;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *FFTFactory(const Pothos::DType &dtype, const size_t numBins, const bool inverse)
{
    #define ifTypeDeclareFactory__(Type) \
        if (dtype == Pothos::DType(typeid(Type))) return new FFT<Type>(numBins, inverse);
    #define ifTypeDeclareFactory(Type) \
        ifTypeDeclareFactory__(std::complex<Type>)
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(kiss_fft_scalar);
    throw Pothos::InvalidArgumentException("FFTFactory("+dtype.toString()+")", "unsupported type");
}
static Pothos::BlockRegistry registerFFT(
    "/comms/fft", &FFTFactory);
