// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstring> //memcpy
#include <iostream>
#include <algorithm> //min/max
#include <complex>
#include <cstdint>

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
 * |default [-1.0, 1.0]
 *
 * |param symbolWidth [Symbol Width] The number of samples per preamble symbol.
 * Each symbol in the preamble will be duplicated by the specified symbol width.
 * Note: this is not the same as the samples per symbol used in data modulation,
 * and is is typically many times greater for synchronization purposes.
 * |default 20
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
 * |factory /blocks/frame_sync(dtype)
 * |setter setPreamble(preamble)
 * |setter setSymbolWidth(symbolWidth)
 * |setter setFrameStartId(frameStartId)
 * |setter setFrameEndId(frameEndId)
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
        _symbolWidth(0)
    {
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type));
        this->setupOutput("freq", typeid(RealType));
        this->setupOutput("corr", typeid(RealType));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getPreamble));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setSymbolWidth));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getSymbolWidth));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, setFrameEndId));
        this->registerCall(this, POTHOS_FCN_TUPLE(FrameSync, getFrameEndId));

        this->setSymbolWidth(20); //initial update
        this->setPreamble(std::vector<Type>(1, 1)); //initial update
        this->setFrameStartId("frameStart"); //initial update
        this->setFrameEndId("frameEnd"); //initial update
    }

    void setPreamble(const std::vector<Type> preamble)
    {
        if (preamble.empty()) throw Pothos::InvalidArgumentException("FrameSync::setPreamble()", "preamble cannot be empty");
        _preamble = preamble;
    }

    std::vector<Type> getPreamble(void) const
    {
        return _preamble;
    }

    void setSymbolWidth(const size_t width)
    {
        if (width == 0) throw Pothos::InvalidArgumentException("FrameSync::setSymbolWidth()", "symbol width cannot be 0");
        _symbolWidth = width;
    }

    size_t getSymbolWidth(void) const
    {
        return _symbolWidth;
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

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        const size_t minReq = (_preamble.size()+1)*_symbolWidth;
        if (inPort->elements() < minReq)
        {
            inPort->setReserve(minReq);
            return;
        }

        auto in = inPort->buffer().template as<const Type *>();

        RealType deltaFc, scale;
        this->processFreqSync(in, deltaFc, scale);

        //using scale and frequency offset, calculate correlation
        Type L = 0;
        RealType freqCorr = 0;
        auto frameSyms = in;
        for (size_t i = 0; i < _preamble.size(); i++)
        {
            const auto sym = std::conj(_preamble[i]);
            for (size_t j = 0; j < _symbolWidth; j++)
            {
                auto frameSym = *frameSyms++;
                L += sym*frameSym*std::polar<RealType>(1.0/scale, freqCorr);
                freqCorr += deltaFc;
            }
        }

        if (_inActiveRegion)
        {
            if (std::abs(L) < _deactivationThresh)
            {
                std::cout << "become deactive " <<std::abs(L) << " old _maxL " << _maxL << "\n";
                _inActiveRegion = false;
                _maxL = 0;
            }
        }
        else
        {
            if (std::abs(L) > _activationThresh)
            {
                _inActiveRegion = true;
                _maxL = 0;
            }
        }

        if (_inActiveRegion and std::abs(L) > _maxL)
        {
            _countSinceMax = 0;
            _maxL = std::abs(L);
            _phaseInc = deltaFc;
            _phaseAccum = -std::arg(L);
            /*
            std::cout << "in[0] " << in[0] << std::endl;
            std::cout << "deltaFc " << deltaFc << std::endl;
            std::cout << "L " << L << std::endl;
            */
        }

        auto out = outPort->buffer().template as<Type *>();
        out[0] = in[0]*std::polar<RealType>(1.0, _phaseAccum);
        _phaseAccum += _phaseInc;

        if (_countSinceMax == size_t(_symbolWidth*(0.8+_preamble.size())))
        {
            outPort->postLabel(Pothos::Label("phase_est", Pothos::Object(_phaseAccum), 0));
        }
        _countSinceMax++;

        if (_phaseAccum > +2*M_PI) _phaseAccum -= 2*M_PI;
        if (_phaseAccum < -2*M_PI) _phaseAccum += 2*M_PI;
        inPort->consume(1);
        outPort->produce(1);

        this->output("freq")->buffer().template as<RealType *>()[0] = _phaseInc;
        this->output("corr")->buffer().template as<RealType *>()[0] = std::abs(L);
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
        _maxL = 0;
        _inActiveRegion = false;
        _activationThresh = _symbolWidth*(_preamble.size()-0.5);
        _deactivationThresh = _symbolWidth*(_preamble.size()-1);
        std::cout << "_preamble.size() " << _preamble.size() << std::endl;
        std::cout << "_symbolWidth " << _symbolWidth << std::endl;
        std::cout << "_activationThresh " << _activationThresh << std::endl;
        std::cout << "_deactivationThresh " << _deactivationThresh << std::endl;
    }

private:

    void processFreqSync(const Type *in, RealType &deltaFc, RealType &scale)
    {
        //when searching for frame start, first calculate the frequency offset
        //and while we are at it, estimate the envelope of the frame
        Type K = 0;
        RealType env = 0;
        auto lastSymBar = in + _symbolWidth*_preamble.size();
        auto lastSym = lastSymBar - _symbolWidth;
        const auto numFreqSyncSamps = size_t(_symbolWidth*0.8);
        for (size_t i = 0; i < numFreqSyncSamps; i++)
        {
            env += std::abs(lastSym[i]) + std::abs(lastSymBar[i]);
            K += lastSym[i] * std::conj(lastSymBar[i]);
        }
        deltaFc = std::arg(K)/numFreqSyncSamps;

        //normalize to the amplitude of the actual symbol
        env = env/(numFreqSyncSamps*2);
        scale = env/std::abs(_preamble.back());
    }

    std::string _frameStartId;
    std::string _frameEndId;
    std::vector<Type> _preamble;
    size_t _symbolWidth;

    RealType _phaseInc;
    RealType _phaseAccum;

    RealType _maxL;
    unsigned long long _countSinceMax;
    bool _inActiveRegion;
    RealType _activationThresh;
    RealType _deactivationThresh;
};

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
