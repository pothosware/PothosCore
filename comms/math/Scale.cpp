// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <iostream>
#include <complex>
#include <algorithm> //min/max
#include <type_traits>

template <typename T, typename U>
typename std::enable_if<std::is_floating_point<T>::value, U>::type
scale(const T &factor, const U &in)
{
    return factor*in;
}

template <typename T, typename U>
typename std::enable_if<std::is_integral<T>::value, U>::type
scale(const T &factor, const U &in)
{
    auto tmp = factor*T(in);
    return U(tmp >> (sizeof(T)*4));
}

template <typename T, typename U>
typename std::enable_if<std::is_floating_point<T>::value, std::complex<U>>::type
scale(const T &factor, const std::complex<U> &in)
{
    return factor*in;
}

template <typename T, typename U>
typename std::enable_if<std::is_integral<T>::value, std::complex<U>>::type
scale(const T &factor, const std::complex<U> &in)
{
    auto tmp = factor*std::complex<T>(in);
    auto real = U(tmp.real() >> (sizeof(T)*4));
    auto imag = U(tmp.imag() >> (sizeof(T)*4));
    return std::complex<U>(real, imag);
}

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
template <typename Type, typename Bigger>
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
        double scale = 1.0;
        if (std::is_integral<Bigger>::value)
        {
            scale = std::ldexp(scale, sizeof(Bigger)*4);
        }
        _factorScaled = Bigger(scale*_factor);
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
            out[i] = scale(_factorScaled, in[i]);
        }

        //produce and consume on 0th ports
        inPort->consume(elems);
        outPort->produce(elems);
    }

private:
    double _factor;
    Bigger _factorScaled;
    std::string _labelId;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *scaleFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory_(type, bigger) \
        if (dtype == Pothos::DType(typeid(type))) return new Scale<type, bigger>();
    #define ifTypeDeclareFactory(type, bigger) \
        ifTypeDeclareFactory_(type, bigger) \
        ifTypeDeclareFactory_(std::complex<type>, bigger)
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
