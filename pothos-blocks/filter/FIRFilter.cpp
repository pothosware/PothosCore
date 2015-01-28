// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <cassert>
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
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param tapsType[Taps Type] The type of the filter taps (real or complex).
 * |option [Real] "REAL"
 * |option [Complex] "COMPLEX"
 *
 * |param decim[Decimation] The downsampling factor.
 * |default 1
 * |widget SpinBox(minimum=1)
 *
 * |param interp[Interpolation] The upsampling factor.
 * |default 1
 * |widget SpinBox(minimum=1)
 *
 * |param taps The FIR filter taps used in convolution.
 * Manually enter or paste in FIR filter taps or leave this entry blank
 * and use the FIR Designer taps signal to configure the filter taps at runtime.
 * |default [1.0]
 *
 * |factory /blocks/fir_filter(dtype, tapsType)
 * |setter setTaps(taps)
 * |setter setDecimation(decim)
 * |setter setInterpolation(interp)
 **********************************************************************/
template <typename InType, typename OutType, typename TapsType>
class FIRFilter : public Pothos::Block
{
public:
    FIRFilter(void):
        M(1),
        L(1),
        K(1)
    {
        this->setupInput(0, typeid(InType));
        this->setupOutput(0, typeid(OutType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setDecimation));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getDecimation));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setInterpolation));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getInterpolation));
        this->setTaps(std::vector<TapsType>(1, TapsType(1))); //initial update
    }

    void setTaps(const std::vector<TapsType> &taps)
    {
        if (taps.empty()) throw Pothos::InvalidArgumentException("FIRFilter::setTaps()", "taps cannot be empty");
        _taps = taps;
        this->updateInternals();
    }

    std::vector<TapsType> getTaps(void) const
    {
        return _taps;
    }

    void setDecimation(const size_t decim)
    {
        if (decim == 0) throw Pothos::InvalidArgumentException("FIRFilter::setDecimation()", "decimation cannot be 0");
        M = decim;
        this->updateInternals();
    }

    size_t getDecimation(void) const
    {
        return M;
    }

    void setInterpolation(const size_t interp)
    {
        if (interp == 0) throw Pothos::InvalidArgumentException("FIRFilter::setInterpolation()", "interpolation cannot be 0");
        L = interp;
        this->updateInternals();
    }

    size_t getInterpolation(void) const
    {
        return L;
    }

    //! always use a circular buffer to avoid discontinuity over sliding window
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        //require the minimum number of input elements to produce at least 1 output
        const auto inputRequire = (M + (K-1));
        if (inPort->elements() < inputRequire)
        {
            inPort->setReserve(inputRequire);
            return;
        }

        //how many iterations?
        const auto N = std::min((inPort->elements()-(K-1))/M, outPort->elements()/L);

        //grab pointers
        auto x = inPort->buffer().template as<const InType *>() + (K-1);
        auto y = outPort->buffer().template as<OutType *>();

        //for each decimated input
        for (size_t n = 0; n < N; n++)
        {
            //interpolation loop
            for (size_t j = 0; j < L; j++)
            {
                //convolution loop
                OutType y_n = 0;
                for (size_t k = 0; k < _interpTaps[j].size(); k++)
                {
                    y_n += _interpTaps[j][k] * x[n*M-k];
                }
                y[j+n*L] = y_n;
            }
        }

        //consume decimated, produce interpolated
        //K-1 elements are left in the input buffer for filter history
        inPort->consume(N*M);
        outPort->produce(N*L);
    }

    void propagateLabels(const Pothos::InputPort *port)
    {
        auto outputPort = this->output(0);
        for (const auto &label : port->labels())
        {
            outputPort->postLabel(label.toAdjusted(L, M));
        }
    }

private:

    void updateInternals(void)
    {
        //https://en.wikipedia.org/wiki/Upsampling
        assert(M > 0);
        assert(L > 0);
        assert(not _taps.empty());

        //K is the largest value of k for which h[j+kL] is non-zero
        K = _taps.size()/L + (((_taps.size()%L) == 0)?0:1);
        this->input(0)->setReserve(K+M-1);
        assert(K > 0);

        //Precalculate the taps array for each interpolation index,
        //because the zeros contribute nothing to its dot product calculations.
        _interpTaps.resize(L);
        for (size_t j = 0; j < L; j++)
        {
            _interpTaps[j].clear();
            for (size_t k = 0; k < K; k++)
            {
                const auto i = j+k*L;
                if (i >= _taps.size()) continue;
                _interpTaps[j].push_back(_taps[i]);
            }
        }
    }

    std::vector<TapsType> _taps;
    std::vector<std::vector<TapsType>> _interpTaps;
    size_t M, L, K;
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
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("FIRFilterFactory("+dtype.toString()+")", "unsupported types");
}
static Pothos::BlockRegistry registerFIRFilter(
    "/blocks/fir_filter", &FIRFilterFactory);
