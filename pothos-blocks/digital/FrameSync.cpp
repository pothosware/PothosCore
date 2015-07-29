// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memcpy
#include <iostream>
#include <algorithm> //min/max
#include <complex>
#include <cstdint>

static const size_t NUM_LENGTH_BITS = 16;

/***********************************************************************
 * |PothosDoc Frame Sync
 *
 * The Frame Sync block ..
 *
 * |category /Digital
 * |keywords preamble frame sync timing offset recover
 *
 * |param dtype[Data Type] The input data type consumed by the slicer.
 * |widget DTypeChooser(cfloat=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param preamble A vector of symbols representing the preamble.
 * |default [1, 1, -1]
 * |option [Barker Code 2] \[1, -1\]
 * |option [Barker Code 3] \[1, 1, -1\]
 * |option [Barker Code 4] \[1, 1, -1, 1\]
 * |option [Barker Code 5] \[1, 1, 1, -1, 1\]
 *
 * |param symbolWidth [Symbol Width] The number of samples per preamble symbol.
 * This value should correspond to the symbol width used in the frame inserter block.
 * |default 20
 * |units samples
 *
 * |param dataWidth [Data Width] The number of samples per data symbol.
 * |default 4
 * |units samples
 *
 * |param frameStartId[Frame Start ID] The label ID that marks the first symbol of frame data.
 * |default "frameStart"
 * |widget StringEntry()
 *
 * |param frameEndId[Frame End ID] The label ID that marks the last symbol of frame data.
 * |default "frameEnd"
 * |widget StringEntry()
 *
 * |param phaseOffsetID[Phase Offset ID] The label ID used to resync phase downstream.
 * The phase offset label specifies the phase offset as a double value in radians.
 * A downstream block may use this to reset its internal phase tracking loop.
 * The phase offset label will not be produced when the label is not specified.
 * |default "phaseOffset"
 * |widget StringEntry()
 * |preview valid
 *
 * |factory /blocks/frame_sync(dtype)
 * |setter setPreamble(preamble)
 * |setter setSymbolWidth(symbolWidth)
 * |setter setDataWidth(dataWidth)
 * |setter setFrameStartId(frameStartId)
 * |setter setFrameEndId(frameEndId)
 * |setter setPhaseOffsetID(phaseOffsetID)
 **********************************************************************/
template <typename Type>
class FrameSync : public Pothos::Block
{
    typedef typename Type::value_type RealType;

public:
    static Block *make(void)
    {
        return new FrameSync();
    }

    FrameSync(void):
        _symbolWidth(0),
        _dataWidth(0),
        _syncWordWidth(0),
        _frameWidth(0)
    {
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type));
        this->setupOutput("freq", typeid(RealType));
        this->setupOutput("corr", typeid(RealType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setSymbolWidth));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getSymbolWidth));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setDataWidth));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getDataWidth));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setFrameEndId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getFrameEndId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setPhaseOffsetID));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getPhaseOffsetID));

        this->setSymbolWidth(20); //initial update
        this->setDataWidth(4); //initial update
        this->setPreamble(std::vector<Type>(1, 1)); //initial update
        this->setFrameStartId("frameStart"); //initial update
        this->setFrameEndId("frameEnd"); //initial update
        this->setPhaseOffsetID("phaseOffset"); //initial update
    }

    void setPreamble(const std::vector<Type> preamble)
    {
        if (preamble.empty()) throw Pothos::InvalidArgumentException("FrameSync::setPreamble()", "preamble cannot be empty");
        _preamble = preamble;
        this->updateSettings();
    }

    std::vector<Type> getPreamble(void) const
    {
        return _preamble;
    }

    void setSymbolWidth(const size_t width)
    {
        if (width == 0) throw Pothos::InvalidArgumentException("FrameSync::setSymbolWidth()", "symbol width cannot be 0");
        _symbolWidth = width;
        this->updateSettings();
    }

    size_t getSymbolWidth(void) const
    {
        return _symbolWidth;
    }

    void setDataWidth(const size_t width)
    {
        if (width < 2) throw Pothos::InvalidArgumentException("FrameSync::setDataWidth()", "data width should be at least 2 samples per symbol");
        _dataWidth = width;
        this->updateSettings();
    }

    size_t getDataWidth(void) const
    {
        return _dataWidth;
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

    void setPhaseOffsetID(std::string id)
    {
        _phaseOffsetId = id;
    }

    std::string getPhaseOffsetID(void) const
    {
        return _phaseOffsetId;
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);
        auto in = inPort->buffer().template as<const Type *>();
        auto out = outPort->buffer().template as<Type *>();

        /***************************************************************
         * A frame was found, consume remaining frame until payload
         **************************************************************/
        if (_remainingFrame != 0)
        {
            auto N = std::min(_remainingFrame, inPort->elements());
            inPort->consume(N);
            _remainingFrame -= N;
            _phase += _phaseInc*N;
            return;
        }

        /***************************************************************
         * A frame was found, produce payload until complete
         * Correct the payload phase and resample for data width
         **************************************************************/
        if (_payloadElems != 0)
        {
            auto N = std::min(_payloadElems, inPort->elements());
            auto widthOut = 1;//_dataWidth
            N = std::min(N, outPort->elements()/widthOut);
            inPort->consume(N);
            _payloadElems -= N;

            size_t produced = 0;
            for (size_t i = 0; i < N; i+=widthOut)
            {
                out[produced++] = in[i+widthOut/2]*std::polar<RealType>(_scaleAtMax, _phase);
                _phase += _phaseInc*widthOut;
            }
            outPort->produce(produced);
            return;
        }

        /***************************************************************
         * Correlation search for a new frame
         **************************************************************/
        if (inPort->elements() < _frameWidth)
        {
            inPort->setReserve(_frameWidth);
            return;
        }

        auto N = inPort->elements()-_frameWidth+1;
        auto minOutElems = this->workInfo().minAllOutElements;
        auto freqBuff = this->output("freq")->buffer().template as<RealType *>();
        auto corrBuff = this->output("corr")->buffer().template as<RealType *>();
        size_t produced = 0;

        if (N == 0) return;
        if (minOutElems == 0) return;

        while (N != produced and minOutElems != produced)
        {
            RealType deltaFc, scale;
            this->processFreqSync(in+produced, deltaFc, scale);

            RealType phaseOff; size_t corrPeak;
            this->processSyncWord(in+produced, deltaFc, scale, phaseOff, corrPeak);

            if (corrPeak > _maxCorrPeak)
            {
                _maxCorrPeak = corrPeak;
                _countSinceMax = 0;
                _deltaFcMax = deltaFc;
                _phaseOffMax = phaseOff;
                _scaleAtMax = scale;
            }

            _countSinceMax++;
            if (_maxCorrPeak > (_syncWordWidth*0.85) and _countSinceMax > (_syncWordWidth*0.5))
            {
                std::cout << "PEAK FOUND \n";
                std::cout << " _countSinceMax = " << _countSinceMax << std::endl;
                std::cout << " _maxCorrPeak = " << _maxCorrPeak << std::endl;
                std::cout << " _deltaFcMax = " << _deltaFcMax << std::endl;
                std::cout << " _phaseOffMax = " << _phaseOffMax << std::endl;
                std::cout << " _scaleAtMax = " << _scaleAtMax << std::endl;
                _phaseInc = _deltaFcMax;
                _phase = _phaseOffMax + _phaseInc*_countSinceMax;
                size_t length;
                this->processLenBits(in-_countSinceMax+produced, _deltaFcMax, _scaleAtMax, _phaseOffMax, length);
                std::cout << " length = " << length << std::endl;
                _remainingFrame = _frameWidth-_countSinceMax;
                _payloadElems = length*_dataWidth;
                _maxCorrPeak = 0;
                //TODO labels... out
                break;
            }

            out[produced] = in[produced]*std::polar<RealType>(scale, _phase);
            freqBuff[produced] = deltaFc;
            corrBuff[produced] = corrPeak;

            _phase += _phaseInc;
            //if (_phase > +2*M_PI) _phase -= 2*M_PI;
            //if (_phase < -2*M_PI) _phase += 2*M_PI;

            produced++;
        }

        inPort->consume(produced);
        outPort->produce(produced);
        this->output("freq")->produce(produced);
        this->output("corr")->produce(produced);
    }

    void propagateLabels(const Pothos::InputPort *port)
    {
        for (const auto label : port->labels())
        {
            this->output(0)->postLabel(label);
        }
    }

    //! always use a circular buffer to avoid discontinuity over sliding window
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

    void activate(void)
    {
        _maxCorrPeak = 0;
        _countSinceMax = 0;
        _phase = 0;
        _phaseInc = 0;
        _payloadElems = 0;
        _remainingFrame = 0;
    }

private:

    void processFreqSync(const Type *in, RealType &deltaFc, RealType &scale);
    void processSyncWord(const Type *in, const RealType &deltaFc, const RealType &scale, RealType &phaseOff, size_t &corrPeak);
    void processLenBits(const Type *in, const RealType &deltaFc, const RealType &scale, const RealType &phaseOff, size_t &length);

    void updateSettings(void)
    {
        _syncWordWidth = _symbolWidth*_dataWidth*_preamble.size();
        _frameWidth = _syncWordWidth+(NUM_LENGTH_BITS*_dataWidth);
    }

    std::string _frameStartId;
    std::string _frameEndId;
    std::string _phaseOffsetId;
    std::vector<Type> _preamble;
    size_t _symbolWidth;
    size_t _dataWidth;
    size_t _syncWordWidth;
    size_t _frameWidth;

    size_t _maxCorrPeak;
    size_t _countSinceMax;
    RealType _deltaFcMax;
    RealType _phaseOffMax;
    RealType _scaleAtMax;
    size_t _remainingFrame;
    size_t _payloadElems;

    RealType _phase;
    RealType _phaseInc;
};

/***********************************************************************
 * Process the frame sync to find the freq offset
 **********************************************************************/
template <typename Type>
void FrameSync<Type>::processFreqSync(const Type *in, RealType &deltaFc, RealType &scale)
{
    const size_t width = _symbolWidth*_dataWidth;
    auto syms = in + width*(_preamble.size()-1);
    const size_t delta = width/2;
    const size_t iterations = width-delta;

    //when searching for frame start, first calculate the frequency offset
    //and while we are at it, estimate the envelope of the frame
    Type K = 0;
    RealType env = 0;
    for (size_t i = 0; i < iterations; i++)
    {
        env += std::abs(syms[i]);
        K += syms[i] * std::conj(syms[i+delta]);
    }
    deltaFc = std::arg(K)/delta;

    //normalize to the amplitude of the actual symbol
    env = env/iterations;
    scale = std::abs(_preamble.back())/env;
}

/***********************************************************************
 * Process the sync word to find the max correlation
 **********************************************************************/
template <typename Type>
void FrameSync<Type>::processSyncWord(const Type *in, const RealType &deltaFc, const RealType &scale, RealType &phaseOff, size_t &corrPeak)
{
    //using scale and frequency offset, calculate correlation
    Type L = 0;
    RealType freqCorr = 0;
    auto frameSyms = in;
    const auto width = _symbolWidth*_dataWidth;
    for (size_t i = 0; i < _preamble.size(); i++)
    {
        const auto sym = std::conj(_preamble[i]);
        for (size_t j = 0; j < width; j++)
        {
            auto frameSym = *frameSyms++;
            L += sym*frameSym*std::polar<RealType>(scale, freqCorr);
            freqCorr += deltaFc;
        }
    }

    //the phase offset at the first point is the angle of L
    phaseOff = -std::arg(L);

    //the correlation peak is the magnitude of L
    corrPeak = size_t(std::abs(L));
}

/***********************************************************************
 * Process the length bits to get a symbol count
 **********************************************************************/
template <typename Type>
void FrameSync<Type>::processLenBits(const Type *in, const RealType &deltaFc, const RealType &scale, const RealType &phaseOff, size_t &length)
{
    length = 0;

    //sample offset to length field + sample offset
    const size_t lengthBitsOff = _syncWordWidth + _dataWidth/2;
    auto lenBits = in + lengthBitsOff;
    RealType freqCorr = phaseOff + deltaFc*(lengthBitsOff);

    //the bit value is the phase difference with the last symbol
    const auto sym = std::conj(_preamble.back());
    for (int i = NUM_LENGTH_BITS-1; i >= 0; i--)
    {
        auto bit = (*lenBits)*std::polar<RealType>(scale, freqCorr)*sym;
        freqCorr += deltaFc*_dataWidth;
        lenBits += _dataWidth;

        auto angle = std::arg(bit);
        if (std::abs(angle) < M_PI/2) length |= (1 << i);
    }
}

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *FrameSyncFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(type) \
        if (dtype == Pothos::DType(typeid(std::complex<type>))) \
            return new FrameSync<std::complex<type>>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    throw Pothos::InvalidArgumentException("FrameSyncFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerFrameSync(
    "/blocks/frame_sync", &FrameSyncFactory);
