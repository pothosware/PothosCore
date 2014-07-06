// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Exception.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/gui/tests", test_eval_expression)
{
    //check that the following does not throw
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto EvalEnvironment = env->findProxy("Pothos/Gui/EvalEnvironment");
    auto evalEnv = EvalEnvironment.callProxy("new");

    const auto result = evalEnv.call<Pothos::Object>("eval", "1 + 2");
    std::cout << "result " << result.convert<int>() << std::endl;

    const auto result2 = evalEnv.call<Pothos::Object>("eval", "DType(\"int32\")");
    std::cout << "result " << result2.convert<Pothos::DType>().toString() << std::endl;
}
