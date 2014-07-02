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

    void eval(const Poco::JSON::Object::Ptr &blockDesc, const std::map<std::string, std::string> &props);

    Poco::JSON::Object::Ptr inspect(void)
    {
        auto block = proxyBlock;
        Poco::JSON::Object::Ptr info = new Poco::JSON::Object();

        info->set("uid", block.call<std::string>("uid"));

        Poco::JSON::Array::Ptr inputPorts = new Poco::JSON::Array();
        for (const auto &name : block.call<std::vector<std::string>>("inputPortNames"))
        {
            Poco::JSON::Object::Ptr portInfo = new Poco::JSON::Object();
            portInfo->set("name", name);
            portInfo->set("isSlot", block.callProxy("input", name).call<bool>("isSlot"));
            inputPorts->add(portInfo);
        }
        info->set("inputPorts", inputPorts);

        Poco::JSON::Array::Ptr outputPorts = new Poco::JSON::Array();
        for (const auto &name : block.call<std::vector<std::string>>("outputPortNames"))
        {
            Poco::JSON::Object::Ptr portInfo = new Poco::JSON::Object();
            portInfo->set("name", name);
            portInfo->set("isSignal", block.callProxy("output", name).call<bool>("isSignal"));
            outputPorts->add(portInfo);
        }
        info->set("outputPorts", outputPorts);

        return info;
    }

private:
    Pothos::Proxy proxyBlock;
    EvalEnvironment &evalEnv;
};

void BlockEval::eval(const Poco::JSON::Object::Ptr &blockDesc, const std::map<std::string, std::string> &props)
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
    proxyBlock = registry.getHandle()->call(path, ctorArgs.data(), ctorArgs.size());
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
}

#include <Pothos/Managed.hpp>

static auto managedBlockEval = Pothos::ManagedClass()
    .registerConstructor<BlockEval, EvalEnvironment &>()
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, eval))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, inspect))
    .commit("Pothos/Gui/BlockEval");
