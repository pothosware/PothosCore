// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Types.h>
#include <complex>
#include <iostream>

/***********************************************************************
 * |PothosDoc FIR Filter
 *
 * The FIR filter convolves an input element stream from port 0 with
 * the filter taps to produce an output element stream on port 0.
 *
 * https://en.wikipedia.org/wiki/Finite_impulse_response
 *
 * |category /Filter
 * |keywords fir filter taps highpass lowpass bandpass
 *
 * |param dtype The data type of the input and output element stream.
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
 * |param tapsType[Taps Type] The type of the filter taps (real or complex).
 * |option [Real] "REAL"
 * |option [Complex] "COMPLEX"
 *
 * |param taps The FIR filter taps used in convolution.
 * Manually enter or paste in FIR filter taps or leave this entry blank
 * and use the FIR Designer taps signal to configure the filter taps at runtime.
 * |default []
 *
 * |factory /blocks/fir_filter(dtype, tapsType)
 * |setter setTaps(taps)
 **********************************************************************/
template <typename InType, typename OutType, typename TapsType>
class FIRFilter : public Pothos::Block
{
public:
    FIRFilter(void)
    {
        this->setupInput(0, typeid(InType));
        this->setupOutput(0, typeid(OutType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getTaps));
    }

    void setTaps(const std::vector<TapsType> &taps)
    {
        this->input(0)->setReserve(taps.size());
        _taps = taps;
    }

    std::vector<TapsType> getTaps(void) const
    {
        return _taps;
    }

    //! always use a circular buffer to avoid discontinuity
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        //grab pointers
        auto x = inPort->buffer().template as<const InType *>();
        auto y = outPort->buffer().template as<OutType *>();

        //how many elements?
        const auto M = _taps.size() - 1;
        const auto N = std::min(inPort->elements()-M, outPort->elements());

        //convolution of taps across input
        for (size_t n = 0; n < N; n++)
        {
            OutType y_n = 0;
            for (size_t i = 0; i <= M; i++)
            {
                y_n += _taps[i] * x[M+n-i];
            }
            y[n] = y_n;
        }

        //produce/consume N elements
        //M elements are left in the input buffer for filter history
        inPort->consume(N);
        outPort->produce(N);
    }

private:
    std::vector<TapsType> _taps;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *FIRFilterFactory(const Pothos::DType &dtype, const std::string &tapsType)
{
    #define ifTypeDeclareFactory__(Type, tapsTypeVal, TapsType) \
        if (dtype == Pothos::DType(typeid(Type)) and tapsType == tapsTypeVal) return new FIRFilter<Type, Type, TapsType>();
    #define ifTypeDeclareFactory(type) \
        ifTypeDeclareFactory__(type, "REAL", type) \
        ifTypeDeclareFactory__(std::complex<type>, "REAL", type) \
        ifTypeDeclareFactory__(std::complex<type>, "COMPLEX", std::complex<type>)
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(Poco::Int64);
    ifTypeDeclareFactory(Poco::Int32);
    ifTypeDeclareFactory(Poco::Int16);
    ifTypeDeclareFactory(Poco::Int8);
    throw Pothos::InvalidArgumentException("FIRFilterFactory("+dtype.toString()+")", "unsupported types");
}
static Pothos::BlockRegistry registerFIRFilter(
    "/blocks/fir_filter", &FIRFilterFactory);
