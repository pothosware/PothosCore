// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/QFormat.hpp>
#include <iostream>
#include <complex>
#include <cmath>
#include <algorithm> //min/max

using Pothos::Util::fromQ;
using Pothos::Util::floatToQ;

/***********************************************************************
 * |PothosDoc Rotate
 *
 * Perform a complex phase rotation operation on every input element.
 *
 * out[n] = in[n] * exp(j*phase)
 *
 * |category /Math
 * |keywords math phase multiply
 *
 * |param dtype[Data Type] The data type used in the arithmetic.
 * |widget DTypeChooser(cint=1, cfloat=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param phase[Phase] The phase rotation in radians.
 * |units radians
 * |default 0.0
 *
 * |param labelId[Label ID] A optional label ID that can be used to change the phase rotator.
 * Upstream blocks can pass a configurable phase rotator along with the stream data.
 * The rotate block searches input labels for an ID match and interprets the label data as the new phase rotator.
 * |preview valid
 * |default ""
 * |widget StringEntry()
 * |tab Labels
 *
 * |factory /comms/rotate(dtype)
 * |setter setPhase(phase)
 * |setter setLabelId(labelId)
 **********************************************************************/
template <typename Type, typename QType>
class Rotate : public Pothos::Block
{
public:
    Rotate(void):
        _phase(0.0)
    {
        this->registerCall(this, POTHOS_FCN_TUPLE(Rotate, setPhase));
        this->registerCall(this, POTHOS_FCN_TUPLE(Rotate, getPhase));
        this->registerCall(this, POTHOS_FCN_TUPLE(Rotate, setLabelId));
        this->registerCall(this, POTHOS_FCN_TUPLE(Rotate, getLabelId));
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type));
    }

    void setPhase(const double phase)
    {
        _phase = phase;
        _phasor = floatToQ<QType>(std::polar(1.0, phase));
    }

    double getPhase(void) const
    {
        return _phase;
    }

    void setLabelId(const std::string &id)
    {
        _labelId = id;
    }

    std::string getLabelId(void) const
    {
        return _labelId;
    }

    void work(void)
    {
        //number of elements to work with
        auto elems = this->workInfo().minElements;
        if (elems == 0) return;

        //get pointers to in and out buffer
        auto inPort = this->input(0);
        auto outPort = this->output(0);
        auto in = inPort->buffer().template as<const Type *>();
        auto out = outPort->buffer().template as<Type *>();

        //check the labels for rotation phase
        if (not _labelId.empty()) for (const auto &label : inPort->labels())
        {
            if (label.index >= elems) break; //ignore labels past input bounds
            if (label.id == _labelId)
            {
                //only set scale-phase when the label is at the front
                if (label.index == 0)
                {
                    this->setPhase(label.data.template convert<double>());
                }
                //otherwise stop processing before the next label
                //on the next call, this label will be index 0
                else
                {
                    elems = label.index;
                    break;
                }
            }
        }

        //perform scale operation
        for (size_t i = 0; i < elems; i++)
        {
            const QType tmp = _phasor*QType(in[i]);
            out[i] = fromQ<Type>(tmp);
        }

        //produce and consume on 0th ports
        inPort->consume(elems);
        outPort->produce(elems);
    }

private:
    double _phase;
    QType _phasor;
    std::string _labelId;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *rotateFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory_(type, qtype) \
        if (dtype == Pothos::DType(typeid(type))) return new Rotate<type, qtype>();
    #define ifTypeDeclareFactory(type, qtype) \
        ifTypeDeclareFactory_(std::complex<type>, std::complex<qtype>)
    ifTypeDeclareFactory(double, double);
    ifTypeDeclareFactory(float, float);
    ifTypeDeclareFactory(int64_t, int64_t);
    ifTypeDeclareFactory(int32_t, int64_t);
    ifTypeDeclareFactory(int16_t, int32_t);
    ifTypeDeclareFactory(int8_t, int16_t);
    throw Pothos::InvalidArgumentException("rotateFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerRotate(
    "/comms/rotate", &rotateFactory);
