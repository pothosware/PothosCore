// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc Transform Signal
 *
 * The transform signal block performs a user-specified expression evaluation
 * on an input slot and produces the evaluation result on an output signal.
 * The input slot is named "transform". The output signal is named "triggered".
 * The arguments from the "transform" slot must be primitive types.
 *
 * |category /Utility
 * |keywords signal slot eval expression
 *
 * |param expr[Expression] The expression to re-evaluate for each slot event.
 * The expression may have any number of arguments named a0, a1, a2... etc.
 * The number of arguments should match the number of arguments from the upstream signal.
 * |default "log2(a0)"
 * |widget StringEntry()
 *
 * |factory /blocks/transform_signal()
 * |setter setExpression(expr)
 **********************************************************************/
class TransformSignal : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new TransformSignal();
    }

    TransformSignal(void)
    {
        this->registerSlot("transform"); //opaqueCallHandler
        this->registerSignal("triggered");
        this->registerCall(this, POTHOS_FCN_TUPLE(TransformSignal, setExpression));
        this->registerCall(this, POTHOS_FCN_TUPLE(TransformSignal, getExpression));

        //create eval environment
        auto env = Pothos::ProxyEnvironment::make("managed");
        _EvalEnvironment = env->findProxy("Pothos/Util/EvalEnvironment");
    }

    void setExpression(const std::string &expr)
    {
        _expr = expr;
    }

    std::string getExpression(void) const
    {
        return _expr;
    }

    //Pothos is cool because you can have advanced overload hooks like this,
    //but dont use this block as a good example of signal and slots usage.
    Pothos::Object opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
    {
        if (name == "transform")
        {
            this->callVoid("triggered", this->peformEval(inputArgs, numArgs));
            return Pothos::Object();
        }
        return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);
    }

    //make an instance of the the evaluator in pothos-utils
    //register the input arguments as constants
    //then evaluate the user-specified expression
    Pothos::Object peformEval(const Pothos::Object *inputArgs, const size_t numArgs)
    {
        auto evalEnv = _EvalEnvironment.callProxy("make");
        for (size_t i = 0; i < numArgs; i++)
        {
            evalEnv.callVoid("registerConstantObj", "a"+std::to_string(i), inputArgs[i]);
        }
        return evalEnv.call<Pothos::Object>("eval", _expr);
    }

private:
    std::string _expr;
    Pothos::Proxy _EvalEnvironment;
};

static Pothos::BlockRegistry registerTransformSignal(
    "/blocks/transform_signal", &TransformSignal::make);
