// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Util/QFormat.hpp>
#include <cstdint>
#include <iostream>
#include <complex>
#include <algorithm> //min/max

using Pothos::Util::fromQ;
using Pothos::Util::floatToQ;

/***********************************************************************
 * |PothosDoc Scale
 *
 * Perform a multiply by scalar operation on every input element.
 *
 * out[n] = in[n] * factor
 *
 * |category /Math
 * |keywords math scale multiply factor gain
 *
 * |param dtype[Data Type] The data type used in the arithmetic.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float64"
 * |preview disable
 *
 * |param factor[Factor] The multiplication scale factor.
 * |default 0.0
 *
 * |param labelId[Label ID] A optional label ID that can be used to change the scale factor.
 * Upstream blocks can pass a configurable scale factor along with the stream data.
 * The scale block searches input labels for an ID match and interprets the label data as the new scale factor.
 * |preview valid
 * |default ""
 * |widget StringEntry()
 * |tab Labels
 *
 * |factory /comms/scale(dtype)
 * |setter setFactor(factor)
 * |setter setLabelId(labelId)
 **********************************************************************/
template <typename Type, typename QType, typename ScaleType>
class Scale : public Pothos::Block
{
public:
    Scale(void):
        _factor(0.0)
    {
        this->registerCall(this, POTHOS_FCN_TUPLE(Scale, setFactor));
        this->registerCall(this, POTHOS_FCN_TUPLE(Scale, getFactor));
        this->registerCall(this, POTHOS_FCN_TUPLE(Scale, setLabelId));
        this->registerCall(this, POTHOS_FCN_TUPLE(Scale, getLabelId));
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type));
    }

    void setFactor(const double factor)
    {
        _factor = factor;
        _factorScaled = floatToQ<ScaleType>(_factor);
    }

    double getFactor(void) const
    {
        return _factor;
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

        //check the labels for scale factors
        if (not _labelId.empty()) for (const auto &label : inPort->labels())
        {
            if (label.index >= elems) break; //ignore labels past input bounds
            if (label.id == _labelId)
            {
                //only set scale-factor when the label is at the front
                if (label.index == 0)
                {
                    this->setFactor(label.data.template convert<double>());
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
            const QType tmp = _factorScaled*QType(in[i]);
            out[i] = fromQ<Type>(tmp);
        }

        //produce and consume on 0th ports
        inPort->consume(elems);
        outPort->produce(elems);
    }

private:
    double _factor;
    ScaleType _factorScaled;
    std::string _labelId;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *scaleFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory_(type, qtype, scaleType) \
        if (dtype == Pothos::DType(typeid(type))) return new Scale<type, qtype, scaleType>();
    #define ifTypeDeclareFactory(type, qtype) \
        ifTypeDeclareFactory_(type, qtype, qtype) \
        ifTypeDeclareFactory_(std::complex<type>, std::complex<qtype>, qtype)
    ifTypeDeclareFactory(double, double);
    ifTypeDeclareFactory(float, float);
    ifTypeDeclareFactory(int64_t, int64_t);
    ifTypeDeclareFactory(int32_t, int64_t);
    ifTypeDeclareFactory(int16_t, int32_t);
    ifTypeDeclareFactory(int8_t, int16_t);
    throw Pothos::InvalidArgumentException("scaleFactory("+dtype.toString()+")", "unsupported type");
}

static Pothos::BlockRegistry registerScale(
    "/comms/scale", &scaleFactory);
