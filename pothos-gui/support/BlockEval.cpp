// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "EvalEnvironment.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <string>
#include <vector>
#include <map>

class BlockEval
{
public:
    BlockEval(EvalEnvironment &env):
        evalEnv(env)
    {
        return;
    };

    Pothos::Proxy eval(const Poco::JSON::Object::Ptr &blockDesc, const std::map<std::string, std::string> &props);

    static Poco::JSON::Object::Ptr inspectBlock(const Pothos::Proxy &block)
    {
        Poco::JSON::Object::Ptr info = new Poco::JSON::Object();

        info->set("uid", block.call<std::string>("uid"));

        Poco::JSON::Array inputNames;
        for (const auto &name : block.call<std::vector<std::string>>("inputPortNames"))
        {
            inputNames.add(name);
        }
        info->set("inputNames", inputNames);

        Poco::JSON::Array outputNames;
        for (const auto &name : block.call<std::vector<std::string>>("outputPortNames"))
        {
            outputNames.add(name);
        }
        info->set("outputNames", outputNames);

        return info;
    }

private:
    EvalEnvironment &evalEnv;
};

Pothos::Proxy BlockEval::eval(const Poco::JSON::Object::Ptr &blockDesc, const std::map<std::string, std::string> &props)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto path = blockDesc->getValue<std::string>("path");

    //load up the constructor args
    std::vector<Pothos::Proxy> ctorArgs;
    for (auto arg : *blockDesc->getArray("args"))
    {
        const auto propKey = arg.extract<std::string>();
        const auto propVal = props.at(propKey);
        const auto obj = evalEnv.eval(propVal);
        ctorArgs.push_back(env->convertObjectToProxy(obj));
    }

    //create the block
    auto proxyBlock = registry.getHandle()->call(path, ctorArgs.data(), ctorArgs.size());
    //proxyBlock.callVoid("setName", block->getId().toStdString());

    //make the calls
    for (auto call : *blockDesc->getArray("calls"))
    {
        const auto callObj = call.extract<Poco::JSON::Object::Ptr>();
        const auto callName = callObj->get("name").extract<std::string>();
        std::vector<Pothos::Proxy> callArgs;
        for (auto arg : *callObj->getArray("args"))
        {
            const auto propKey = arg.extract<std::string>();
            const auto propVal = props.at(propKey);
            const auto obj = evalEnv.eval(propVal);
            callArgs.push_back(env->convertObjectToProxy(obj));
        }
        proxyBlock.getHandle()->call(callName, callArgs.data(), callArgs.size());
    }

    return proxyBlock;
}

#include <Pothos/Managed.hpp>

static auto managedBlockEval = Pothos::ManagedClass()
    .registerConstructor<BlockEval, EvalEnvironment &>()
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, eval))
    .registerStaticMethod(POTHOS_FCN_TUPLE(BlockEval, inspectBlock))
    .commit("Pothos/Gui/BlockEval");
