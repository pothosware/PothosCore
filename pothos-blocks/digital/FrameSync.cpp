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
 * The Frame Sync block decodes a PHY synchronization header,
 * and forwards frame payload data to the output port.
 * The synchronization header can be used to establish frequency,
 * phase, and sample timing offset, and frame boundaries.
 *
 * <h2>Output modes</h2>
 *
 * The frame sync block can be used with downstream synchronization blocks
 * such as carrier tracking, matched filtering, and timing recovery.
 * However, the frame sync block does have its own built-in
 * carrier offset and sample timing compensation based on
 * the offsets discovered by the frame header search.
 * Use external synchronization blocks that track phase and offset drift
 * for long payloads or when there is significant drift within the payload.
 *
 * <h3>Raw payload output</h3>
 *
 * Once the frame is discovered, the raw input symbols are forwarded
 * to the output for the entire frame payload length.
 * In this mode, the next downstream block may track carrier phase.
 * Also, the phase offset label can be used to reset the starting phase
 * to help the loop synchronize at the start of the payload.
 *
 * A Costas loop may be used track carrier phase:<br />
 * <a href="https://en.wikipedia.org/wiki/Costas_loop">https://en.wikipedia.org/wiki/Costas_loop</a>
 *
 * <h3>Phase compensation</h3>
 *
 * Using the discovered carrier frequency and phase offset,
 * the frame sync will apply phase compensation to the payload symbols.
 * Supposing that the waveform was QPSK, the next downstream block
 * may be a matched filter to aid in symbol timing recovery.
 *
 * A Root-raised-cosing filter may aid in symbol recovery:<br />
 * <a href="https://en.wikipedia.org/wiki/Root-raised-cosine_filter">https://en.wikipedia.org/wiki/Root-raised-cosine_filter</a>
 *
 * <h3>Re-sample timing</h3>
 *
 * In addition to the phase compensation noted above,
 * the frame sync will use the discovered timing offset
 * to re-sample the payload to the original symbol rate.
 * The next downstream block may perform symbol detection
 * to remap the recovered symbols into data bits.
 *
 * |category /Digital
 * |keywords preamble frame sync timing offset recover
 *
 * |param dtype[Data Type] The input data type consumed by the slicer.
 * |widget DTypeChooser(cfloat=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param outputMode[Output Mode] The output mode for payload symbols.
 * |default "RAW"
 * |option [Raw symbols] "RAW"
 * |option [Phase correction] "PHASE"
 * |option [Timing recovery] "TIMING"
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
 * |param inputThreshold [Input Threshold] The activation threshold for raw input samples.
 * The frame search algorithm will discard input below this threshold, effectively noise.
 * |default 0.01
 *
 * |param frameStartId[Frame Start ID] The label ID that marks the first symbol of frame data.
 * The label data will contain the length of the payload as encoded by the frame inserter.
 * |default "frameStart"
 * |widget StringEntry()
 * |preview valid
 * |tab Labels
 *
 * |param frameEndId[Frame End ID] The label ID that marks the last symbol of frame data.
 * The frame end label will not be produced when the label ID is not specified.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 * |tab Labels
 *
 * |param phaseOffsetID[Phase Offset ID] The label ID used to resync phase downstream.
 * The phase offset label specifies the phase offset as a double value in radians.
 * A downstream block may use this to reset its internal phase tracking loop.
 * The phase offset label will not be produced when the label ID is not specified.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 * |tab Labels
 *
 * |factory /blocks/frame_sync(dtype)
 * |setter setOutputMode(outputMode)
 * |setter setPreamble(preamble)
 * |setter setSymbolWidth(symbolWidth)
 * |setter setDataWidth(dataWidth)
 * |setter setFrameStartId(frameStartId)
 * |setter setFrameEndId(frameEndId)
 * |setter setPhaseOffsetID(phaseOffsetID)
 * |setter setInputThreshold(inputThreshold)
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
        _frameWidth(0),
        _inputThreshold(0)
    {
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setOutputMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getOutputMode));
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
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setInputThreshold));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getInputThreshold));

        this->setOutputMode("RAW");
        this->setSymbolWidth(20); //initial update
        this->setDataWidth(4); //initial update
        this->setPreamble(std::vector<Type>(1, 1)); //initial update
        this->setFrameStartId("frameStart"); //initial update
        this->setFrameEndId(""); //initial update
        this->setPhaseOffsetID(""); //initial update
        this->setInputThreshold(0.01); //initial update
    }

    void setOutputMode(const std::string &mode)
    {
        if (mode == "RAW"){}
        else if (mode == "PHASE"){}
        else if (mode == "TIMING"){}
        else throw Pothos::InvalidArgumentException("FrameSync::setOutputMode("+mode+")", "unknown output mode");
        _outputModeRaw = (mode == "RAW");
        _outputModePhase = (mode == "PHASE");
        _outputModeTiming = (mode == "TIMING");
        _outputModeStr = mode;
    }

    std::string getOutputMode(void) const
    {
        return _outputModeStr;
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

    void setInputThreshold(const RealType threshold)
    {
        if (threshold < 0) throw Pothos::InvalidArgumentException("FrameSync::setInputThreshold()", "threshold should be non-negative");
        _inputThreshold = threshold;
    }

    RealType getInputThreshold(void) const
    {
        return _inputThreshold;
    }

    void work(void);

    void propagateLabels(const Pothos::InputPort *port)
    {
        for (const auto label : port->labels())
        {
            //labels from input currently discarded
            //this->output(0)->postLabel(label);
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
        _remainingPayload = 0;
        _remainingHeader = 0;
    }

private:

    void processEnvelope(const Type *in, RealType &scale);
    void processFreqSync(const Type *in, RealType &deltaFc);
    void processSyncWord(const Type *in, const RealType &deltaFc, const RealType &scale, RealType &phaseOff, size_t &corrPeak);
    void processLenBits(const Type *in, const RealType &deltaFc, const RealType &scale, const RealType &phaseOff, size_t &firstBit, size_t &length);

    void updateSettings(void)
    {
        _syncWordWidth = _symbolWidth*_dataWidth*_preamble.size();
        _frameWidth = _syncWordWidth+(NUM_LENGTH_BITS*_dataWidth);
        _corrMagThresh = size_t(_syncWordWidth*0.7);
        _corrDurThresh = size_t(_syncWordWidth*0.5);
    }

    //output mode
    std::string _outputModeStr;
    bool _outputModeRaw;
    bool _outputModePhase;
    bool _outputModeTiming;

    //configuration
    std::string _frameStartId;
    std::string _frameEndId;
    std::string _phaseOffsetId;
    std::vector<Type> _preamble;
    size_t _symbolWidth; //width of a preamble symbol
    size_t _dataWidth; //width of a data dymbol
    size_t _syncWordWidth; //preamble sync portion width
    size_t _frameWidth; //full frame header width
    size_t _corrMagThresh; //minimum required correlation magnitude threshold
    size_t _corrDurThresh; //minimum required correlation duration threshold
    RealType _inputThreshold; //minimum required input activation threshold

    //values at last max correlation peak
    size_t _maxCorrPeak;
    size_t _countSinceMax;
    RealType _deltaFcMax;
    RealType _phaseOffMax;
    RealType _scaleAtMax;

    //track header and payload after frame found
    size_t _remainingHeader;
    size_t _remainingPayload;

    //calculated output offset corrections
    RealType _phase;
    RealType _phaseInc;
};

/***********************************************************************
 * Work implementation
 **********************************************************************/
template <typename Type>
void FrameSync<Type>::work(void)
{
    auto inPort = this->input(0);
    auto outPort = this->output(0);
    auto in = inPort->buffer().template as<const Type *>();
    auto out = outPort->buffer().template as<Type *>();

    /***************************************************************
     * A frame was found, consume remaining frame until payload
     **************************************************************/
    if (_remainingHeader != 0)
    {
        auto N = std::min(_remainingHeader, inPort->elements());
        _phase += _phaseInc*N;
        _remainingHeader -= N;
        inPort->consume(N);
        if (_remainingHeader == 0) std::cout << "header consumed\n";
        return;
    }

    /***************************************************************
     * Produce payload with no compensation
     **************************************************************/
    else if (_remainingPayload != 0 and _outputModeRaw)
    {
        const auto N = std::min(_remainingPayload, this->workInfo().minElements);

        for (size_t i = 0; i < N; i++)
        {
            out[i] = in[i]*_scaleAtMax;
        }

        _remainingPayload -= N;
        inPort->consume(N);
        outPort->produce(N);
        return;
    }

    /***************************************************************
     * Produce payload with phase compensation
     **************************************************************/
    else if (_remainingPayload != 0 and _outputModePhase)
    {
        const auto N = std::min(_remainingPayload, this->workInfo().minElements);

        for (size_t i = 0; i < N; i++)
        {
            out[i] = in[i]*std::polar<RealType>(_scaleAtMax, _phase);
            _phase += _phaseInc;
        }

        _remainingPayload -= N;
        inPort->consume(N);
        outPort->produce(N);
        return;
    }

    /***************************************************************
     * Produce payload with timing compensation
     **************************************************************/
    else if (_remainingPayload != 0 and _outputModeTiming)
    {
        auto N = std::min(_remainingPayload, inPort->elements());
        N = std::min(N/_dataWidth, outPort->elements());
        if (N == 0) inPort->setReserve(_dataWidth);

        for (size_t i = 0; i < N; i++)
        {
            const auto sym = in[i*_dataWidth];
            out[i] = sym*std::polar<RealType>(_scaleAtMax, _phase);
            _phase += _phaseInc*_dataWidth;
        }

        const size_t consumed = N*_dataWidth;
        _remainingPayload -= consumed;
        inPort->consume(consumed);
        outPort->produce(N);
        if (_remainingPayload == 0) std::cout << "payload forwarded\n";
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
    const auto N = inPort->elements()-_frameWidth+1;

    for (size_t i = 0; i < N; i++)
    {
        //TODO think about when the give up and reset state
        if (_countSinceMax > _corrDurThresh)
        {
            _maxCorrPeak = 0;
        }

        //calculate the scaling value, and check for consistent envelope
        RealType scale;
        this->processEnvelope(in+i, scale);
        if (scale == 0) continue;

        //calculate the frequency offset as if this was the frame start
        RealType deltaFc;
        this->processFreqSync(in+i, deltaFc);

        //use the frequency offset to calculate the correlation value
        RealType phaseOff; size_t corrPeak;
        this->processSyncWord(in+i, deltaFc, scale, phaseOff, corrPeak);

        //if this correlation value is larger, record the state
        if (corrPeak > _maxCorrPeak)
        {
            _maxCorrPeak = corrPeak;
            _countSinceMax = 0;
            _deltaFcMax = deltaFc;
            _phaseOffMax = phaseOff;
            _scaleAtMax = scale;
        }
        _countSinceMax++;

        //check if the peak is above the threshold and we have not found
        //a greater correlation peak within the specified duration threshold
        if (_maxCorrPeak < _corrMagThresh) continue;
        if (_countSinceMax < _corrDurThresh) continue;

        //print summary
        std::cout << "PEAK FOUND \n";
        std::cout << " _countSinceMax = " << _countSinceMax << std::endl;
        std::cout << " _maxCorrPeak = " << _maxCorrPeak << std::endl;
        std::cout << " _deltaFcMax = " << _deltaFcMax << std::endl;
        std::cout << " _phaseOffMax = " << _phaseOffMax << std::endl;
        std::cout << " _scaleAtMax = " << _scaleAtMax << std::endl;

        _maxCorrPeak = 0; //reset for next time

        //now that the frame was found, process the length field
        //and determine sample offset (used in timing recovery mode)
        size_t firstBit, length;
        this->processLenBits(in+i-_countSinceMax, _deltaFcMax, _scaleAtMax, _phaseOffMax, firstBit, length);
        if (length == 0) continue; //this is probably a false frame detection

        //print length results
        std::cout << " length = " << length << std::endl;
        std::cout << " firstBit = " << firstBit << std::endl;
        std::cout << " sampOffset = " << (int(firstBit)-int(_syncWordWidth)) << std::endl;

        //initialize carrier recovery compensation for use in the
        //remaining header and payload sections of the work routine
        _remainingHeader = firstBit+(NUM_LENGTH_BITS*_dataWidth)-_countSinceMax;
        _remainingPayload = length*_dataWidth;
        _phaseInc = _deltaFcMax;
        _phase = _phaseOffMax + _phaseInc*_countSinceMax;
        std::cout << " _remainingHeader = " << _remainingHeader << std::endl;
        std::cout << " _remainingPayload = " << _remainingPayload << std::endl;

        //Label width is specified based on the output mode.
        //Width may be divided down by an upstream time recovery block.
        //The length and label width are used in conjunction to specify
        //the number of elements between start and end labels.
        const size_t labelWidth = _outputModeTiming?1:_dataWidth;

        //produce a phase offset label at the first payload index
        if (not _phaseOffsetId.empty()) outPort->postLabel(
            Pothos::Label(_phaseOffsetId, Pothos::Object(_phase),
            0, labelWidth));

        //produce a start of frame label at the first payload index
        if (not _frameStartId.empty()) outPort->postLabel(
            Pothos::Label(_frameStartId, Pothos::Object(length),
            0, labelWidth));

        //produce an end of frame label at the last payload index
        if (not _frameEndId.empty()) outPort->postLabel(
            Pothos::Label(_frameEndId, Pothos::Object(length),
            (length-1)*labelWidth, labelWidth));

        inPort->consume(i+1);
        return;
    }
    inPort->consume(N);
}

/***********************************************************************
 * Process the envelope of the frame preamble
 **********************************************************************/
template <typename Type>
void FrameSync<Type>::processEnvelope(const Type *in, RealType &scale)
{
    scale = 0;

    //spot check the amplitude near the sync word edges
    if (std::abs(in[_dataWidth]) < _inputThreshold) return;
    if (std::abs(in[_syncWordWidth-_dataWidth]) < _inputThreshold) return;

    //width of a sum across symbol in samples
    const size_t width = _symbolWidth*_dataWidth/2;

    //get a rough average of amplitude at the beginning and end
    RealType sumFirst = 0;
    for (size_t i = 0; i < width; i++)
    {
        sumFirst += std::abs(in[i]);
    }
    sumFirst /= width*std::abs(_preamble.front());

    RealType sumLast = 0;
    for (size_t i = _syncWordWidth-1; i >= _syncWordWidth-width; i--)
    {
        sumLast += std::abs(in[i]);
    }
    sumLast /= width*std::abs(_preamble.back());

    //check for consistent amplitude across the frame
    const auto ratio = sumFirst/sumLast;
    if (ratio > 2 or ratio < 0.5) return;

    //use the two sums to estimate the scale
    scale = 2.0/(sumFirst + sumLast);
}

/***********************************************************************
 * Process the frame sync to find the freq offset
 **********************************************************************/
template <typename Type>
void FrameSync<Type>::processFreqSync(const Type *in, RealType &deltaFc)
{
    //width of a preamble symbol in samples
    const size_t width = _symbolWidth*_dataWidth;

    //offset into the start of the final preamble symbol
    auto syms = in + width*(_preamble.size()-1);

    //difference between any two compare samples
    const size_t delta = width/2;

    //avoid transition edges with padding
    const size_t padding = _dataWidth;
    const size_t begin = padding;
    const size_t end = width - delta - padding;

    //calculate the frequency offset across multiple
    //pairs of samples that are within the same symbol
    Type K = 0;
    for (size_t i = begin; i < end; i++)
    {
        K += syms[i] * std::conj(syms[i+delta]);
    }
    deltaFc = std::arg(K)/delta;
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
void FrameSync<Type>::processLenBits(const Type *in, const RealType &deltaFc, const RealType &scale, const RealType &phaseOff, size_t &firstBit, size_t &length)
{
    firstBit = 0;
    length = 0;

    //the last preamble symbol is used to encode the phase shifts
    const auto sym = std::conj(_preamble.back());

    //use the intentional phase transitions before the
    //length bits to determine the optimal sampling offset
    firstBit = _syncWordWidth + _dataWidth/2;
    RealType firstBitPeak = 0;
    for (size_t i = _syncWordWidth-_dataWidth; i <= _syncWordWidth+_dataWidth; i++)
    {
        auto bit = in[i]*std::polar<RealType>(scale, phaseOff + deltaFc*i)*sym;
        if (bit.real() > firstBitPeak) continue;
        firstBit = i;
        firstBitPeak = bit.real();
    }

    //never found the peak, probably not a frame
    if (firstBitPeak == 0) return;

    //offsets to sampling index of length bits
    auto lenBits = in + firstBit;
    RealType freqCorr = phaseOff + deltaFc*(firstBit);

    //the bit value is the phase difference with the last symbol
    for (int i = NUM_LENGTH_BITS-1; i >= 0; i--)
    {
        auto bit = (*lenBits)*std::polar<RealType>(scale, freqCorr)*sym;
        if (bit.real() > 0) length |= (1 << i);
        freqCorr += deltaFc*_dataWidth;
        lenBits += _dataWidth;
    }

    //top two bits used for timing sync
    length &= 0x3fff;
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
