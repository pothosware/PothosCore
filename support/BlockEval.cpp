// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "BlockEval.hpp"

void BlockEval::eval(const std::string &id, const Poco::JSON::Object::Ptr &blockDesc)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");
    auto path = blockDesc->getValue<std::string>("path");

    //load up the constructor args
    std::vector<Pothos::Proxy> ctorArgs;
    for (auto arg : *blockDesc->getArray("args"))
    {
        const auto propKey = arg.extract<std::string>();
        const auto obj = this->properties[propKey];
        ctorArgs.push_back(env->convertObjectToProxy(obj));
    }

    //create the block
    try
    {
        proxyBlock = registry.getHandle()->call(path, ctorArgs.data(), ctorArgs.size());
    }
    catch (const Pothos::Exception &ex)
    {
        throw Pothos::Exception("BlockEval factory("+path+")", ex);
    }
    proxyBlock.callVoid("setName", id);

    //make the calls
    for (auto call : *blockDesc->getArray("calls"))
    {
        const auto callObj = call.extract<Poco::JSON::Object::Ptr>();
        const auto callName = callObj->get("name").extract<std::string>();
        std::vector<Pothos::Proxy> callArgs;
        for (auto arg : *callObj->getArray("args"))
        {
            const auto propKey = arg.extract<std::string>();
            const auto obj = this->properties[propKey];
            callArgs.push_back(env->convertObjectToProxy(obj));
        }
        try
        {
            proxyBlock.getHandle()->call(callName, callArgs.data(), callArgs.size());
        }
        catch (const Pothos::Exception &ex)
        {
            throw Pothos::Exception("BlockEval call("+callName+")", ex);
        }
    }
}

Poco::JSON::Object::Ptr BlockEval::inspect(void)
{
    auto block = proxyBlock;
    Poco::JSON::Object::Ptr info = new Poco::JSON::Object();

    info->set("uid", block.call<std::string>("uid"));

    //TODO FIXME inspect will fail for topologies ATM, cant query isSignal/isSlot on topology

    Poco::JSON::Array::Ptr inputPorts = new Poco::JSON::Array();
    for (const auto &name : block.call<std::vector<std::string>>("inputPortNames"))
    {
        Poco::JSON::Object::Ptr portInfo = new Poco::JSON::Object();
        portInfo->set("name", name);
        portInfo->set("isSlot", block.callProxy("input", name).call<bool>("isSlot"));
        portInfo->set("size", block.callProxy("input", name).callProxy("dtype").call<unsigned>("size"));
        portInfo->set("dtype", block.callProxy("input", name).callProxy("dtype").call<std::string>("toString"));
        inputPorts->add(portInfo);
    }
    info->set("inputPorts", inputPorts);

    Poco::JSON::Array::Ptr outputPorts = new Poco::JSON::Array();
    for (const auto &name : block.call<std::vector<std::string>>("outputPortNames"))
    {
        Poco::JSON::Object::Ptr portInfo = new Poco::JSON::Object();
        portInfo->set("name", name);
        portInfo->set("isSignal", block.callProxy("output", name).call<bool>("isSignal"));
        portInfo->set("size", block.callProxy("output", name).callProxy("dtype").call<unsigned>("size"));
        portInfo->set("dtype", block.callProxy("output", name).callProxy("dtype").call<std::string>("toString"));
        outputPorts->add(portInfo);
    }
    info->set("outputPorts", outputPorts);

    return info;
}

#include <Pothos/Managed.hpp>

static auto managedBlockEval = Pothos::ManagedClass()
    .registerConstructor<BlockEval, EvalEnvironment &>()
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, evalProperty))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, eval))
    .registerMethod(POTHOS_FCN_TUPLE(BlockEval, inspect))
    .commit("Pothos/Gui/BlockEval");
