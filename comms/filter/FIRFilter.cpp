// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/QFormat.hpp>
#include <cstdint>
#include <complex>
#include <cassert>
#include <cstring> //memset, memcpy
#include <iostream>
#include <algorithm> //min/max

using Pothos::Util::fromQ;
using Pothos::Util::floatToQ;

/***********************************************************************
 * |PothosDoc FIR Filter
 *
 * The FIR filter convolves an input element stream from port 0 with
 * the filter taps to produce an output element stream on port 0.
 *
 * <a href="https://en.wikipedia.org/wiki/Finite_impulse_response">
 * https://en.wikipedia.org/wiki/Finite_impulse_response</a>
 *
 * <h2>Burst support</h2>
 *
 * The FIR filter supports bursts that are properly labeled within a stream.
 * When a burst is encountered, the FIR filter will flush it out without
 * consuming or convolving with any of the samples from the next burst.
 *
 * A burst or frame can be represented in one of two ways:
 * <ol>
 * <li>Using a start of frame label that contains the length in elements.
 * The end of the burst index is considered to be label.index + length * label.width - 1.</li>
 * <li>Or using an end of frame label on the final element of the burst.
 * The end of the burst index is considered to be label.index + label.width - 1.</li>
 * </ol>
 *
 * |category /Filter
 * |keywords fir filter taps highpass lowpass bandpass
 * |alias /blocks/fir_filter
 *
 * |param dtype[Data Type] The data type of the input and output element stream.
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
 * |param waitTaps[Wait Taps] Wait for the taps to be set before allowing operation.
 * Use this mode when taps are set exclusively at runtime by the setTaps() slot.
 * |default false
 * |preview valid
 * |option [Enabled] true
 * |option [Disabled] false
 *
 * |param frameStartId[Frame Start ID] The label ID to mark the first element of a burst.
 * When the start frame ID is specified and the start frame label contains an element length,
 * the FIR filter will flush out the remainder of the burst without consuming the next burst.
 * An empty string (default) disables this feature.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 * |tab Labels
 *
 * |param frameEndId[Frame End ID] The label ID to mark the last element of a burst.
 * Rather than using a start frame label with a length, when the end frame ID is specified,
 * the FIR filter will flush out the remainder of the burst without consuming the next burst.
 * An empty string (default) disables this feature.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 * |tab Labels
 *
 * |factory /comms/fir_filter(dtype, tapsType)
 * |setter setTaps(taps)
 * |setter setDecimation(decim)
 * |setter setInterpolation(interp)
 * |setter setWaitTaps(waitTaps)
 * |setter setFrameStartId(frameStartId)
 * |setter setFrameEndId(frameEndId)
 **********************************************************************/
template <typename InType, typename OutType, typename TapsType, typename BiggerType, typename BiggerTapsType>
class FIRFilter : public Pothos::Block
{
public:
    FIRFilter(void):
        M(1),
        L(1),
        K(1),
        _inputRequire(1),
        _waitTapsMode(false),
        _waitTapsArmed(false),
        _eobSampsLeft(0)
    {
        this->setupInput(0, typeid(InType));
        this->setupOutput(0, typeid(OutType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setDecimation));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getDecimation));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setInterpolation));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getInterpolation));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setWaitTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getWaitTaps));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, setFrameEndId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FIRFilter, getFrameEndId));
        this->setTaps(std::vector<TapsType>(1, TapsType(1))); //initial update
    }

    void setWaitTaps(const bool waitTaps)
    {
        _waitTapsMode = waitTaps;
    }

    bool getWaitTaps(void) const
    {
        return _waitTapsMode;
    }

    void setTaps(const std::vector<TapsType> &taps)
    {
        if (taps.empty()) throw Pothos::InvalidArgumentException("FIRFilter::setTaps()", "taps cannot be empty");
        _taps = taps;
        _waitTapsArmed = false; //got taps
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

    void setFrameStartId(std::string id)
    {
        _frameStartId = id;
    }

    std::string getFrameStartId(void) const
    {
        return _frameStartId;
    }

    void setFrameEndId(std::string id)
    {
        _frameEndId = id;
    }

    std::string getFrameEndId(void) const
    {
        return _frameEndId;
    }

    //! always use a circular buffer to avoid discontinuity over sliding window
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

    void activate(void)
    {
        _waitTapsArmed = _waitTapsMode;
        _eobSampsLeft = 0;
    }

    void work(void)
    {
        if (_waitTapsArmed) return;
        auto inPort = this->input(0);
        auto outPort = this->output(0);
        auto inputAvailable = inPort->elements();
        if (inputAvailable == 0) return;

        /***************************************************************
         * search for burst labels and record the next end of burst
         **************************************************************/
        if (_eobSampsLeft == 0) for (const auto &label : inPort->labels())
        {
            if (not _frameStartId.empty() and label.id == _frameStartId and label.data.canConvert(typeid(size_t)))
            {
                const auto length = label.data.template convert<size_t>();
                _eobSampsLeft = label.index + length*label.width;
                break;
            }
            else if (not _frameEndId.empty() and label.id == _frameEndId)
            {
                _eobSampsLeft = label.index + label.width;
                break;
            }
        }

        /***************************************************************
         * check that the available input is sufficient
         **************************************************************/
        //in burst mode, make sure input available stops at the end
        if (_eobSampsLeft != 0)
        {
            if (_eobSampsLeft <= inputAvailable)
            {
                inputAvailable = _eobSampsLeft;
            }
            else
            {
                inPort->setReserve(_eobSampsLeft);
                return;
            }
        }

        //otherwise insufficient input for the regular streaming mode
        else if (inputAvailable < _inputRequire)
        {
            inPort->setReserve(_inputRequire);
            return;
        }

        //normally we don't enforce a requirement unless there is a problem
        inPort->setReserve(0);

        /***************************************************************
         * Special input buffer to flush the burst
         **************************************************************/
        auto inBuff = inPort->buffer();
        inBuff.length = inputAvailable*sizeof(InType);
        if (_eobSampsLeft != 0 and _eobSampsLeft < _inputRequire)
        {
            const size_t numBytesCopy = _eobSampsLeft*sizeof(InType);
            Pothos::BufferChunk flushBuff(typeid(InType), _eobSampsLeft + K - 1);
            std::memcpy(flushBuff.as<void *>(), inBuff.template as<const void *>(), numBytesCopy);
            std::memset(flushBuff.as<char *>() + numBytesCopy, 0, flushBuff.length-numBytesCopy);
            inBuff = flushBuff;
        }

        /***************************************************************
         * Normal FIR filter operation
         **************************************************************/
        //how many iterations?
        const auto N = std::min((inBuff.elements()-(K-1))/M, outPort->elements()/L);

        //grab pointers
        auto x = inBuff.template as<const InType *>() + (K-1);
        auto y = outPort->buffer().template as<OutType *>();

        //for each decimated input
        for (size_t n = 0; n < N; n++)
        {
            //interpolation loop
            for (size_t j = 0; j < L; j++)
            {
                //convolution loop
                BiggerType y_n = 0;
                for (size_t k = 0; k < _interpTaps[j].size(); k++)
                {
                    y_n += _interpTaps[j][k] * BiggerType(x[n*M-k]);
                }
                y[j+n*L] = fromQ<OutType>(y_n);
            }
        }

        //consume decimated, produce interpolated
        //K-1 elements are left in the input buffer for filter history
        if (_eobSampsLeft != 0) _eobSampsLeft -= N*M;
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
                _interpTaps[j].push_back(floatToQ<BiggerTapsType>(_taps[i]));
            }
        }

        //require the minimum number of input elements to produce at least 1 output
        _inputRequire = (M + (K-1));
    }

    std::vector<TapsType> _taps;
    std::vector<std::vector<BiggerTapsType>> _interpTaps;
    size_t M, L, K, _inputRequire;
    bool _waitTapsMode;
    bool _waitTapsArmed;
    std::string _frameStartId;
    std::string _frameEndId;
    size_t _eobSampsLeft;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *FIRFilterFactory(const Pothos::DType &dtype, const std::string &tapsType)
{
    #define ifTypeDeclareFactory__(Type, tapsTypeVal, TapsType, BiggerType, BiggerTapsType) \
        if (dtype == Pothos::DType(typeid(Type)) and tapsType == tapsTypeVal) return new FIRFilter<Type, Type, TapsType, BiggerType, BiggerTapsType>();
    #define ifTypeDeclareFactory(type, bigger) \
        ifTypeDeclareFactory__(type, "REAL", double, bigger, bigger) \
        ifTypeDeclareFactory__(std::complex<type>, "REAL", double, std::complex<bigger>, bigger) \
        ifTypeDeclareFactory__(std::complex<type>, "COMPLEX", std::complex<double>, std::complex<bigger>, std::complex<bigger>)
    ifTypeDeclareFactory(double, double);
    ifTypeDeclareFactory(float, float);
    ifTypeDeclareFactory(int64_t, int64_t);
    ifTypeDeclareFactory(int32_t, int64_t);
    ifTypeDeclareFactory(int16_t, int32_t);
    ifTypeDeclareFactory(int8_t, int16_t);
    throw Pothos::InvalidArgumentException("FIRFilterFactory("+dtype.toString()+")", "unsupported types");
}
static Pothos::BlockRegistry registerFIRFilter(
    "/comms/fir_filter", &FIRFilterFactory);

static Pothos::BlockRegistry registerFIRFilterOldPath(
    "/blocks/fir_filter", &FIRFilterFactory);
