// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "TopologyEngine/TopologyEngine.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/MD5Engine.h>
#include <Poco/Logger.h>

//! helper to convert the port info vector into JSON for serialization of the block
static Poco::JSON::Array::Ptr portInfosToJSON(const std::vector<Pothos::PortInfo> &infos)
{
    Poco::JSON::Array::Ptr array = new Poco::JSON::Array();
    for (const auto &info : infos)
    {
        Poco::JSON::Object::Ptr portInfo = new Poco::JSON::Object();
        portInfo->set("name", info.name);
        portInfo->set("isSigSlot", info.isSigSlot);
        portInfo->set("size", info.dtype.size());
        portInfo->set("dtype", info.dtype.toString());
        array->add(portInfo);
    }
    return array;
}

/*!
 * Get a hash that uniquely identifies all configuration about this block.
 * Example: path, ID, property values, affinity zone, process id.
 * The hash can be used to check for changes for handling re-evaluation.
 */
static std::string makeGraphBlockConfigHash(GraphBlock *block, const Pothos::ProxyEnvironment::Sptr &env)
{
    Poco::MD5Engine md5;

    //unique block stuff
    md5.update(block->getBlockDescPath());
    md5.update(block->getId().toStdString());

    //affinity and process
    md5.update(env->getUniquePid());
    md5.update(block->getAffinityZone().toStdString());

    //block properties
    for (const auto &propKey : block->getProperties())
    {
        md5.update(propKey.toStdString());
        md5.update(block->getPropertyValue(propKey).toStdString());
    }

    return Poco::DigestEngine::digestToHex(md5.digest());
}

Pothos::Proxy TopologyEngine::evalGraphBlock(GraphBlock *block)
{
    block->clearBlockErrorMsgs();

    //try to get access to a remote environment object
    Pothos::ProxyEnvironment::Sptr env;
    Pothos::Proxy evalEnv;
    if (block->isDisplayWidget())
    {
        //display widgets must run in the gui process
        //TODO this doesnt really support hierarchies + affinity zones
        //eventually, we want only the block with the widget to be local
        env = Pothos::ProxyEnvironment::make("managed");
        evalEnv = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");
    }
    else POTHOS_EXCEPTION_TRY
    {
        env = this->getEnvironmentFromZone(block->getAffinityZone());
        evalEnv = this->getEvalEnvironment(block->getAffinityZone());
    }
    POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
    {
        block->addBlockErrorMsg(QString::fromStdString(ex.displayText()));
        return Pothos::Proxy();
    }

    //check the cache before re-evaluating
    const auto thisHash = makeGraphBlockConfigHash(block, env);
    if (_idToBlockEval[block->getId()].first == thisHash)
    {
        this->setupPorts(block);
        return _idToBlockEval[block->getId()].second;
    }
    _idToBlockEval.erase(block->getId());
    _idToPortDesc.erase(block->getId());

    //create a new block evaluator on the server
    auto BlockEval = env->findProxy("Pothos/Util/BlockEval");
    auto blockEval = BlockEval.callProxy("new", evalEnv);

    //validate the id
    if (block->getId().isEmpty())
    {
        block->addBlockErrorMsg(tr("Error: empty ID"));
    }

    //evaluate the properties
    bool hasError = false;
    for (const auto &propKey : block->getProperties())
    {
        const auto val = block->getPropertyValue(propKey).toStdString();
        try
        {
            auto obj = blockEval.callProxy("evalProperty", propKey.toStdString(), val);
            block->setPropertyTypeStr(propKey, obj.call<std::string>("getTypeString"));
            block->setPropertyErrorMsg(propKey, "");
        }
        catch (const Pothos::Exception &ex)
        {
            block->setPropertyErrorMsg(propKey, QString::fromStdString(ex.message()));
            hasError = true;
        }
    }

    //property errors -- cannot continue
    if (hasError)
    {
        block->addBlockErrorMsg(tr("Error: cannot evaluate this block with property errors"));
        return Pothos::Proxy();
    }

    //evaluate the block and load its port info
    try
    {
        blockEval.callProxy("eval", block->getId().toStdString(), block->getBlockDesc());
        auto proxyBlock = blockEval.callProxy("getProxyBlock");
        _idToPortDesc.emplace(block->getId(), std::make_pair(
            portInfosToJSON(proxyBlock.call<std::vector<Pothos::PortInfo>>("inputPortInfo")),
            portInfosToJSON(proxyBlock.call<std::vector<Pothos::PortInfo>>("outputPortInfo"))));
    }

    //parser errors report
    catch(const Pothos::Exception &ex)
    {
        poco_error(Poco::Logger::get("PothosGui.TopologyEngine.evalGraphBlock"), ex.displayText());
        block->addBlockErrorMsg(QString::fromStdString(ex.message()));
    }

    //update the cache and return
    _idToBlockEval[block->getId()] = std::make_pair(thisHash, blockEval);
    this->setupPorts(block);
    return blockEval;
}

void TopologyEngine::setupPorts(GraphBlock *block)
{
    if (_idToPortDesc.count(block->getId()) == 0) return;
    block->setPortDesc(_idToPortDesc[block->getId()].first, _idToPortDesc[block->getId()].second);
}
