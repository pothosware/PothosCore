// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Format.h>
#include <cctype> //toupper
#include <string>
#include <map>
#include <set>
#include <iostream>

/***********************************************************************
 * |PothosDoc Evaluator
 *
 * The evaluator block performs a user-specified expression evaluation
 * on input slot(s) and produces the evaluation result on an output signal.
 * The input slots are user-defined. The output signal is named "triggered".
 * The arguments from the input slots must be primitive types.
 *
 * |category /Utility
 * |keywords signal slot eval expression
 *
 * |param vars[Variables] A list of named variables to use in the expression.
 * Each variable corresponds to settings slot on the transform block.
 * Example: ["foo", "bar"] will create the slots "setFoo" and "setBar".
 * |default ["val"]
 *
 * |param expr[Expression] The expression to re-evaluate for each slot event.
 * An expression contains combinations of variables, constants, and math functions.
 * Example: log2(foo)/bar
 *
 * <p><b>Multi-argument slots:</b> Upstream blocks may pass multiple arguments to a slot.
 * Each argument will be available to the expression suffixed by its argument index.
 * For example, suppose that the slot "setBaz" has two arguments,
 * then the following expression would use both arguments: "baz0 + baz1"</p>
 *
 * |default "log2(val)"
 * |widget StringEntry()
 *
 * |factory /blocks/evaluator(vars)
 * |setter setExpression(expr)
 **********************************************************************/
class Evaluator : public Pothos::Block
{
public:
    static Block *make(const std::vector<std::string> &varNames)
    {
        return new Evaluator(varNames);
    }

    Evaluator(const std::vector<std::string> &varNames)
    {
        for (const auto &name : varNames)
        {
            if (name.empty()) continue;
            const auto slotName = Poco::format("set%s%s", std::string(1, std::toupper(name.front())), name.substr(1));
            _slotNameToVarName[slotName] = name;
            this->registerSlot(slotName); //opaqueCallHandler
        }
        this->registerSignal("triggered");
        this->registerCall(this, POTHOS_FCN_TUPLE(Evaluator, setExpression));
        this->registerCall(this, POTHOS_FCN_TUPLE(Evaluator, getExpression));

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
        //check if this is for one of the set value slots
        auto it = _slotNameToVarName.find(name);
        if (it == _slotNameToVarName.end()) return Pothos::Block::opaqueCallHandler(name, inputArgs, numArgs);

        //stash the values from the slot arguments
        for (size_t i = 0; i < numArgs; i++)
        {
            if (numArgs == 1) _varValues[it->second] = inputArgs[i];
            else _varValues[Poco::format("%s%d", it->second, int(i))] = inputArgs[i];
        }
        _varsReady.insert(it->second);

        //check that all slots specified are ready
        for (const auto &pair : _slotNameToVarName)
        {
            if (_varsReady.count(pair.second) == 0) return Pothos::Object();
        }

        //perform the evaluation and emit the result
        this->callVoid("triggered", this->peformEval());
        return Pothos::Object();
    }

    //make an instance of the the evaluator in pothos-utils
    //register the input arguments as constants
    //then evaluate the user-specified expression
    Pothos::Object peformEval(void)
    {
        auto evalEnv = _EvalEnvironment.callProxy("make");
        for (const auto &pair : _varValues)
        {
            evalEnv.callVoid("registerConstantObj", pair.first, pair.second);
        }
        return evalEnv.call<Pothos::Object>("eval", _expr);
    }

private:
    std::string _expr;
    std::map<std::string, std::string> _slotNameToVarName;
    std::map<std::string, Pothos::Object> _varValues;
    std::set<std::string> _varsReady;
    Pothos::Proxy _EvalEnvironment;
};

static Pothos::BlockRegistry registerEvaluator(
    "/blocks/evaluator", &Evaluator::make);
