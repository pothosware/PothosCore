// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <complex>
#include <cmath>
#include <iostream>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Envelope Detector
 *
 * The envelope calculator consumes samples from input port 0
 * and computes the envelope using a single pole filter.
 * The input samples can be real or complex integers or floats.
 * The envelope signal is produced on out port 0 as type float.
 *
 * https://en.wikipedia.org/wiki/Envelope_detector
 *
 * |category /Filter
 * |keywords filter envelope attack decay sustain release lookahead
 *
 * |param dtype[Input Type] The data type of the input stream.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param attack The run-up time constant in samples.
 * Single pole filter roll-off constant: gainAttack = exp(-1/attack)
 * |default 10
 * |units samples
 *
 * |param release The decay time constant in samples.
 * Single pole filter roll-off constant: gainRelease = exp(-1/release)
 * |default 10
 * |units samples
 *
 * |param lookahead A configurable input delay to compensate for envelope lag.
 * Without lookahead, the envelope calculation lags behind the input due to filtering.
 * The lookahead compensation adjusts the envelope to match up with the input events.
 * |default 10
 * |units samples
 *
 * |factory /blocks/envelope_detector(dtype)
 * |setter setAttack(attack)
 * |setter setRelease(release)
 * |setter setLookahead(lookahead)
 **********************************************************************/
template <typename InType, typename OutType>
class EnvelopeDetector : public Pothos::Block
{
public:
    EnvelopeDetector(void):
        _envelope(0),
        _attack(0),
        _release(0),
        _lookahead(0),
        _attackGain(0),
        _releaseGain(0),
        _oneMinusAttackGain(0),
        _oneMinusReleaseGain(0)
    {
        this->setupInput(0, typeid(InType));
        this->setupOutput(0, typeid(OutType));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setAttack));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, getAttack));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setRelease));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, getRelease));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setLookahead));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, getLookahead));
    }

    void setAttack(const OutType attack)
    {
        _attack = attack;
        _attackGain = std::exp(-1/attack);
        _oneMinusAttackGain = 1-_attackGain;
    }

    OutType getAttack(void) const
    {
        return _attack;
    }

    void setRelease(const OutType release)
    {
        _release = release;
        _releaseGain = std::exp(-1/release);
        _oneMinusReleaseGain = 1-_releaseGain;
    }

    OutType getRelease(void) const
    {
        return _release;
    }

    void setLookahead(const size_t lookahead)
    {
        _lookahead = lookahead;
    }

    size_t getLookahead(void) const
    {
        return _lookahead;
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        //ensure that the input has lookahead room
        if (inPort->elements() <= _lookahead)
        {
            inPort->setReserve(_lookahead+1);
            return;
        }

        //calculate the work size given the available resources
        const size_t N = std::min(inPort->elements()-_lookahead, outPort->elements());
        if (N == 0) return;

        //cast the input and output buffers
        const auto in = inPort->buffer().template as<const InType *>();
        const auto out = outPort->buffer().template as<OutType *>();

        //perform envelope calculation operation
        for (size_t i = 0; i < N; i++)
        {
            const OutType xn = OutType(std::abs(in[i+_lookahead]));
            if (xn > _envelope)
            {
                _envelope = _attackGain*_envelope + _oneMinusAttackGain*xn;
            }
            else
            {
                _envelope = _releaseGain*_envelope + _oneMinusReleaseGain*xn;
            }
            out[i] = _envelope;
        }

        //produce/consume the work size
        inPort->consume(N);
        outPort->produce(N);
    }

    //! use a circular buffer to implement lookahead window efficiently
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

private:

    OutType _envelope;
    OutType _attack;
    OutType _release;
    size_t _lookahead;
    OutType _attackGain;
    OutType _releaseGain;
    OutType _oneMinusAttackGain;
    OutType _oneMinusReleaseGain;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *EnvelopeDetectorFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory__(InType, OutType) \
        if (dtype == Pothos::DType(typeid(InType))) return new EnvelopeDetector<InType, OutType>();
    #define ifTypeDeclareFactory(type) \
        ifTypeDeclareFactory__(type, float) \
        ifTypeDeclareFactory__(std::complex<type>, float)
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("EnvelopeDetectorFactory("+dtype.toString()+")", "unsupported type");
}
static Pothos::BlockRegistry registerEnvelopeDetector(
    "/blocks/envelope_detector", &EnvelopeDetectorFactory);
