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
 * The envelope detector block calculates the envelope of an input signal
 * and checks the envelope against a configurable threshold to detect
 * and forward useful samples that fall within the activation zone.
 *
 * <h2>Envelope Calculation</h2>
 *
 * The envelope calculator consumes samples from input port 0
 * and computes the envelope using a single pole filter.
 * The input samples can be real or complex integers or floats.
 * The envelope signal is produced on out port 0 as type float.
 *
 * https://en.wikipedia.org/wiki/Envelope_detector
 *
 * <h2>Threshold Operation</h2>
 *
 * The threshold operation compares the calculated envelope
 * against configurable activation and deactivation thresholds
 * to determine regions within the signal that are "useful".
 *
 * The operation forwards samples from input port 0 to output port "th"
 * and adds optional annotation labels to the output stream
 * to indicate threshold activation and deactivation points.
 * The sample forwarding can operate in several modes:
 *
 * <ul>
 * <li><b>PASS</b> - All input samples are passed to the output.
 *  There is a 1:1 ratio between input and output samples.</li>
 * <li><b>ZERO</b> - Samples outside of the threshold are zeroed.
 *  There is a 1:1 ratio between input and output samples.</li>
 * <li><b>DROP</b> - Samples outside of the threshold are dropped.
 *  There is a reduction in the number of output samples.</li>
 * </ul>
 *
 * |category /Filter
 * |keywords filter envelope threshold attack decay sustain release
 *
 * |param dtype[Input Type] The data type of the input stream.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param attack[Attack Duration] The run-up time constant in samples.
 * Single pole filter roll-off constant: alphaAttack = exp(-1/attack)
 * |default 10
 * |units samples
 *
 * |param release[Release Duration] The decay time constant in samples.
 * Single pole filter roll-off constant: alphaRelease = exp(-1/release)
 * |default 10
 * |units samples
 *
 * |param thresholdMode[Threshold Mode] The operation mode for the threshold output data port.
 * |default "PASS"
 * |option [Pass] "PASS"
 * |option [Zero] "ZERO"
 * |option [Drop] "DROP"
 * |tab Threshold
 *
 * |param activationThresh[Activation Threshold] The threshold that the envelope must exceed to activate.
 * |default 0.5
 * |tab Threshold
 *
 * |param deactivationThresh[Deactivation Threshold] The threshold that the envelope must fall-below to deactivate.
 * |default 0.5
 * |tab Threshold
 *
 * |param activationId[Activation ID] The label ID to mark the element that crosses the activation threshold (when inactive).
 * An empty string (default) means that activate labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 * |tab Threshold
 *
 * |param deactivationId[Deactivation ID] The label ID to mark the element that crosses the deactivation threshold (when active).
 * An empty string (default) means that deactivate labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 * |tab Threshold
 *
 * |factory /blocks/envelope_detector(dtype)
 * |setter setAttack(attack)
 * |setter setRelease(release)
 * |setter setActivationThreshold(activationThresh)
 * |setter setDeactivationThreshold(deactivationThresh)
 * |setter setActivationId(activationId)
 * |setter setDeactivationId(deactivationId)
 **********************************************************************/
template <typename InType, typename OutType>
class EnvelopeDetector : public Pothos::Block
{
public:
    EnvelopeDetector(void):
        _envelope(0),
        _attackAlpha(0),
        _releaseAlpha(0),
        _oneMinusAttackAlpha(0),
        _oneMinusReleaseAlpha(0),
        _activationThreshold(0),
        _deactivationThreshold(0),
        _activeState(false),
        _theshOutPort(nullptr)
    {
        this->setupInput(0, typeid(InType));
        this->setupOutput(0, typeid(OutType));
        this->setupOutput("th", typeid(InType), this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setAttack));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setRelease));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setActivationThreshold));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setDeactivationThreshold));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setActivationId));
        this->registerCall(this, POTHOS_FCN_TUPLE(EnvelopeDetector, setDeactivationId));
        _theshOutPort = this->output("th");
    }

    void setAttack(const OutType attack)
    {
        _attackAlpha = std::exp(-1/attack);
        _oneMinusAttackAlpha = 1-_attackAlpha;
    }

    void setRelease(const OutType release)
    {
        _releaseAlpha = std::exp(-1/release);
        _oneMinusReleaseAlpha = 1-_releaseAlpha;
    }

    void setActivationThreshold(const OutType thresh)
    {
        _activationThreshold = thresh;
    }

    void setDeactivationThreshold(const OutType thresh)
    {
        _deactivationThreshold = thresh;
    }

    void setActivationId(const std::string id)
    {
        _activationId = id;
    }

    void setDeactivationId(const std::string id)
    {
        _deactivationId = id;
    }

    void activate(void)
    {
        //reset state before running
        _activeState = false;
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        const auto in = inPort->buffer().template as<const InType *>();
        const auto out = outPort->buffer().template as<OutType *>();
        const size_t N = std::min(inPort->elements(), outPort->elements());
        if (N == 0) return;

        for (size_t i = 0; i < N; i++)
        {
            //envelope calculation
            const OutType xn = OutType(std::abs(in[i]));
            if (xn > _envelope)
            {
                _envelope = _attackAlpha*_envelope + _oneMinusAttackAlpha*xn;
            }
            else
            {
                _envelope = _releaseAlpha*_envelope + _oneMinusReleaseAlpha*xn;
            }
            out[i] = _envelope;

            //threshold operation
            if (not _activeState and _envelope > _activationThreshold)
            {
                _activeState = true;
                if (not _activationId.empty())
                {
                    _theshOutPort->postLabel(Pothos::Label(_activationId, Pothos::Object(), i));
                }
            }
            else if (_activeState and _envelope < _deactivationThreshold)
            {
                _activeState = false;
                if (not _deactivationId.empty())
                {
                    _theshOutPort->postLabel(Pothos::Label(_deactivationId, Pothos::Object(), i));
                }
            }
        }

        inPort->consume(N);
        outPort->produce(N);

        auto buff = inPort->buffer();
        buff.length = N*buff.dtype.size();
        _theshOutPort->postBuffer(buff);
    }

/*
    void propagateLabels(const Pothos::InputPort *port)
    {
        auto outputPort = this->output(0);
        for (const auto &label : port->labels())
        {
        }
    }
    */

private:

    OutType _envelope;
    OutType _attackAlpha;
    OutType _releaseAlpha;
    OutType _oneMinusAttackAlpha;
    OutType _oneMinusReleaseAlpha;
    OutType _activationThreshold;
    OutType _deactivationThreshold;
    std::string _activationId;
    std::string _deactivationId;
    bool _activeState;
    Pothos::OutputPort *_theshOutPort;
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
