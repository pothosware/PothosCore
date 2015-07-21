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
 * |widget DTypeChooser(cfloat=1,cint=1)
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

        auto in = inPort-> buffer().template as<const Type *>();

        //when searching for frame start, first calculate the frequency offset
        Type K = 0;
        auto lastSymBar = in + _symbolWidth*_preamble.size();
        auto lastSym = lastSymBar - _symbolWidth;
        for (size_t i = 0; i < _symbolWidth; i++)
        {
            K += lastSym[i] * std::conj(lastSymBar[i]);
        }

    }

    void propagateLabels(const Pothos::InputPort *)
    {
        //don't propagate here, its done in work()
    }

    //! always use a circular buffer to avoid discontinuity over sliding window
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

private:

    std::string _frameStartId;
    std::string _frameEndId;
    std::vector<Type> _preamble;
    size_t _symbolWidth;
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
    ifTypeDeclareFactory(int16_t);
    throw Pothos::InvalidArgumentException("FrameSyncFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerFrameSync(
    "/blocks/frame_sync", &FrameSyncFactory);
