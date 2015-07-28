// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memcpy
#include <iostream>
#include <algorithm> //min/max
#include <complex>
#include <cstdint>

static const double FREQ_SYNC_WIDTH = 0.8;
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
        _freqSyncWidth(0),
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

        if (not _frameSearch)
        {
            inPort->consume(inPort->elements());
            return;
            //DONE TEST!
        }

        if (inPort->elements() < _frameWidth)
        {
            inPort->setReserve(_frameWidth);
            return;
        }

        auto in = inPort->buffer().template as<const Type *>();

        RealType deltaFc, scale;
        this->processFreqSync(in, deltaFc, scale);

        RealType phaseOff; size_t corrPeak;
        this->processSyncWord(in, deltaFc, scale, phaseOff, corrPeak);

        if (corrPeak > _maxCorrPeak)
        {
            _maxCorrPeak = corrPeak;
            _countSinceMax = 0;
            _deltaFcMax = deltaFc;
            _phaseOffMax = phaseOff;
            _scaleAtMax = scale;
        }

        if (_maxCorrPeak > (_syncWordWidth*0.85) and _countSinceMax > (_syncWordWidth*0.5))
        {
            std::cout << "PEAK FOUND \n";
            std::cout << " _countSinceMax = " << _countSinceMax << std::endl;
            std::cout << " _maxCorrPeak = " << _maxCorrPeak << std::endl;
            std::cout << " _deltaFcMax = " << _deltaFcMax << std::endl;
            std::cout << " _phaseOffMax = " << _phaseOffMax << std::endl;
            std::cout << " _scaleAtMax = " << _scaleAtMax << std::endl;
            //_frameSearch = false;
            size_t length;
            this->processLenBits(in-_countSinceMax, _deltaFcMax, _scaleAtMax, _phaseOffMax, length);
            std::cout << " length = " << length << std::endl;
            _maxCorrPeak = 0;
        }
        _countSinceMax++;

        inPort->consume(1);
        outPort->produce(1);

        auto out = outPort->buffer().template as<Type *>();
        out[0] = in[0];
        this->output("freq")->buffer().template as<RealType *>()[0] = deltaFc;
        this->output("corr")->buffer().template as<RealType *>()[0] = corrPeak;
        this->output("freq")->produce(1);
        this->output("corr")->produce(1);
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
        _frameSearch = true;
    }

private:

    void processFreqSync(const Type *in, RealType &deltaFc, RealType &scale);
    void processSyncWord(const Type *in, const RealType &deltaFc, const RealType &scale, RealType &phaseOff, size_t &corrPeak);
    void processLenBits(const Type *in, const RealType &deltaFc, const RealType &scale, const RealType &phaseOff, size_t &length);

    void updateSettings(void)
    {
        _syncWordWidth = _symbolWidth*_dataWidth*_preamble.size();
        _freqSyncWidth = size_t(_symbolWidth*_dataWidth*FREQ_SYNC_WIDTH);
        _frameWidth = _syncWordWidth+_freqSyncWidth+(NUM_LENGTH_BITS*_dataWidth);
    }

    std::string _frameStartId;
    std::string _frameEndId;
    std::string _phaseOffsetId;
    std::vector<Type> _preamble;
    size_t _symbolWidth;
    size_t _dataWidth;
    size_t _syncWordWidth;
    size_t _freqSyncWidth;
    size_t _frameWidth;

    size_t _maxCorrPeak;
    size_t _countSinceMax;
    RealType _deltaFcMax;
    RealType _phaseOffMax;
    RealType _scaleAtMax;

    bool _frameSearch;
};

/***********************************************************************
 * Process the frame sync to find the freq offset
 **********************************************************************/
template <typename Type>
void FrameSync<Type>::processFreqSync(const Type *in, RealType &deltaFc, RealType &scale)
{
    //when searching for frame start, first calculate the frequency offset
    //and while we are at it, estimate the envelope of the frame
    Type K = 0;
    RealType env = 0;
    auto lastSymBar = in + _syncWordWidth;
    auto lastSym = lastSymBar - _symbolWidth;
    for (size_t i = 0; i < _freqSyncWidth; i++)
    {
        env += std::abs(lastSym[i]) + std::abs(lastSymBar[i]);
        K += lastSym[i] * std::conj(lastSymBar[i]);
    }
    deltaFc = std::arg(K)/_freqSyncWidth;

    //normalize to the amplitude of the actual symbol
    env = env/(_freqSyncWidth*2);
    scale = env/std::abs(_preamble.back());
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
            L += sym*frameSym*std::polar<RealType>(1.0/scale, freqCorr);
            freqCorr += deltaFc;
        }
    }

    //the phase offset at the first point is the angle of L
    phaseOff = std::arg(L);

    //the correlation peak is the magnitude of L
    corrPeak = size_t(std::abs(L));
}

/***********************************************************************
 * Process the length bits to get a symbol count
 **********************************************************************/
template <typename Type>
void FrameSync<Type>::processLenBits(const Type *in, const RealType &deltaFc, const RealType &scale, const RealType &phaseOff, size_t &length)
{
    const size_t lengthBitsOff = _syncWordWidth+_freqSyncWidth;
    auto lenBits = in + lengthBitsOff;
    RealType freqCorr = phaseOff + deltaFc*(lengthBitsOff);

    length = 0;

    const auto sym = std::conj(_preamble.back());

    for (int i = NUM_LENGTH_BITS-1; i >= 0; i--)
    {
        Type sum = 0;
        for (size_t j = 0; j < _dataWidth; j++)
        {
            auto lenBit = *lenBits++;
            sum += lenBit*std::polar<RealType>(1.0/scale, freqCorr);
            freqCorr += deltaFc;
        }
        auto angle = std::arg(sum) - std::arg(sym);
        if (angle > +M_PI) angle -= 2*M_PI;
        if (angle < -M_PI) angle += 2*M_PI;
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
