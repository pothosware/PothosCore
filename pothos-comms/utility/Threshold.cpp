// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <cstdint>
#include <iostream>

/***********************************************************************
 * |PothosDoc Threshold
 *
 * The threshold block compares an input signal from port 0 against
 * configurable activation and deactivation threshold values,
 * and marks the threshold regions with additional stream labels.
 * The stream is forwarded without modification to output port 0.
 *
 * |category /Utility
 * |keywords threshold activate level
 *
 * |param dtype[Data Type] The data type for the input and output streams.
 * |widget DTypeChooser(float=1,int=1)
 * |default "float64"
 * |preview disable
 *
 * |param activationLevel[Activation Level] The threshold level that the input must exceed to activate.
 * |default 0.5
 *
 * |param deactivationLevel[Deactivation Level] The threshold level that the input must fall-below to deactivate.
 * |default 0.5
 *
 * |param activationId[Activation ID] The label ID to mark the element that crosses the activation threshold (when inactive).
 * An empty string (default) means that activate labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 *
 * |param deactivationId[Deactivation ID] The label ID to mark the element that crosses the deactivation threshold (when active).
 * An empty string (default) means that deactivate labels are not produced.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 *
 * |factory /blocks/threshold(dtype)
 * |setter setActivationLevel(activationLevel)
 * |setter setDeactivationLevel(deactivationLevel)
 * |setter setActivationId(activationId)
 * |setter setDeactivationId(deactivationId)
 **********************************************************************/
template <typename Type>
class Threshold : public Pothos::Block
{
public:

    Threshold(void):
        _activationLevel(0),
        _deactivationLevel(0),
        _activeState(false)
    {
        this->setupInput(0, typeid(Type));
        this->setupOutput(0, typeid(Type), this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(Threshold, setActivationLevel));
        this->registerCall(this, POTHOS_FCN_TUPLE(Threshold, getActivationLevel));
        this->registerCall(this, POTHOS_FCN_TUPLE(Threshold, setDeactivationLevel));
        this->registerCall(this, POTHOS_FCN_TUPLE(Threshold, getDeactivationLevel));
        this->registerCall(this, POTHOS_FCN_TUPLE(Threshold, setActivationId));
        this->registerCall(this, POTHOS_FCN_TUPLE(Threshold, getActivationId));
        this->registerCall(this, POTHOS_FCN_TUPLE(Threshold, setDeactivationId));
        this->registerCall(this, POTHOS_FCN_TUPLE(Threshold, getDeactivationId));
    }

    void setActivationLevel(const Type level)
    {
        _activationLevel = level;
    }

    Type getActivationLevel(void) const
    {
        return _activationLevel;
    }

    void setDeactivationLevel(const Type level)
    {
        _deactivationLevel = level;
    }

    Type getDeactivationLevel(void) const
    {
        return _deactivationLevel;
    }

    void setActivationId(const std::string &id)
    {
        _activationId = id;
    }

    std::string getActivationId(void) const
    {
        return _activationId;
    }

    void setDeactivationId(const std::string &id)
    {
        _deactivationId = id;
    }

    std::string getDeactivationId(void) const
    {
        return _deactivationId;
    }

    void activate(void)
    {
        //reset state before running
        _activeState = false;
    }

    void work(void)
    {
        //access ports
        auto inPort = this->input(0);
        auto outPort = this->output(0);

        //get input buffer
        const auto &buff = inPort->buffer();
        const auto in = buff.template as<const Type *>();
        const size_t N = buff.elements();
        if (N == 0) return;

        //check threshold
        for (size_t i = 0; i < N; i++)
        {
            if (not _activeState and in[i] > _activationLevel)
            {
                _activeState = true;
                if (_activationId.empty()) continue;
                outPort->postLabel(Pothos::Label(_activationId, Pothos::Object(), i));
            }
            else if (_activeState and in[i] < _deactivationLevel)
            {
                _activeState = false;
                if (_deactivationId.empty()) continue;
                outPort->postLabel(Pothos::Label(_deactivationId, Pothos::Object(), i));
            }
        }

        //consume input and forward buffer
        inPort->consume(N);
        outPort->postBuffer(buff);
    }

private:

    Type _activationLevel;
    Type _deactivationLevel;
    std::string _activationId;
    std::string _deactivationId;
    bool _activeState;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::Block *ThresholdFactory(const Pothos::DType &dtype)
{
    #define ifTypeDeclareFactory(Type) \
        if (dtype == Pothos::DType(typeid(Type))) return new Threshold<Type>();
    ifTypeDeclareFactory(double);
    ifTypeDeclareFactory(float);
    ifTypeDeclareFactory(int64_t);
    ifTypeDeclareFactory(int32_t);
    ifTypeDeclareFactory(int16_t);
    ifTypeDeclareFactory(int8_t);
    throw Pothos::InvalidArgumentException("ThresholdFactory("+dtype.toString()+")", "unsupported type");
}
static Pothos::BlockRegistry registerThreshold(
    "/blocks/threshold", &ThresholdFactory);
