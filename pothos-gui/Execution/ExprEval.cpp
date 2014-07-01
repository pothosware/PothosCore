// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Execution/ExprEval.hpp"

class ExprEvalEnvironmentImpl : public ExprEvalEnvironment
{
public:

    ExprEvalEnvironmentImpl(const Pothos::ProxyEnvironment::Sptr &env)
    {
        _myEval = env->findProxy("Pothos/Gui/EvalExpression");
    }

    Pothos::Proxy tryEval(const std::string &expression)
    {
        try
        {
            return _myEval.callProxy("eval", expression);
        }
        catch (const Pothos::Exception &ex)
        {
            throw ExprEvalError(ex.displayText());
        }
    }

    void registerConstant(const std::string &key, const std::string &expression)
    {
        //TODO not yet
    }

private:
    Pothos::Proxy _myEval;
};

ExprEvalEnvironment *ExprEvalEnvironment::make(Pothos::RemoteClient &client)
{
    return new ExprEvalEnvironmentImpl(client.makeEnvironment("managed"));
}
