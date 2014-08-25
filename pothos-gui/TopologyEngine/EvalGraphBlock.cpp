// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "EvalBlockCache.hpp"
#include "TopologyEngine/TopologyEngine.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/Logger.h>

Pothos::Proxy TopologyEngine::evalGraphBlock(GraphBlock *block)
{
    block->clearBlockErrorMsgs();

    //try to get access to a remote environment object
    Pothos::ProxyEnvironment::Sptr env;
    Pothos::Proxy evalEnv;
    if (block->isGraphWidget())
    {
        //display widgets must run in the gui process
        //TODO this doesnt really support hierarchies + affinity zones
        //eventually, we want only the block with the widget to be local
        env = Pothos::ProxyEnvironment::make("managed");
        if (not _zoneToEvalEnvironment["gui"])
        {
            _zoneToEvalEnvironment["gui"] = env->findProxy("Pothos/Util/EvalEnvironment").callProxy("new");
        }
        evalEnv = _zoneToEvalEnvironment["gui"];
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

    //grab the cache (or make one)
    auto &cache = _idToBlockEval[block->getId()];
    const bool newCache = not cache;
    if (not cache) cache.reset(new EvalBlockCache(block->getBlockDesc()));

    //load all the current properties into the cache
    for (const auto &propKey : block->getProperties())
    {
        cache->loadCurrentValue(propKey, block->getPropertyValue(propKey));
    }

    //deal with non-critical changes
    if (not newCache and not cache->hasCriticalChange())
    {
        bool setterError = not cache->updateChangedProperties(block);
        if (not setterError) for (const auto &setter : cache->settersChangedList())
        {
            try
            {
                cache->getProxyBlock().callVoid("handleCall", setter);
            }
            catch (const Pothos::Exception &ex)
            {
                poco_error_f2(Poco::Logger::get("PothosGui.TopologyEngine.evalGraphBlock"),
                    "cached block call('%s'): %s", setter->getValue<std::string>("name"), ex.displayText());
                setterError = true;
                break;
            }
        }
        if (not setterError)
        {
            cache->loadCacheIntoBlock(block);
            cache->markUpdateDone();
            return cache->getProxyBlock();
        }
    }

    //create a new block evaluator on the server
    auto BlockEval = env->findProxy("Pothos/Util/BlockEval");
    auto blockEval = BlockEval.callProxy("new", evalEnv);
    cache->setProxyBlock(blockEval);

    //validate the id
    if (block->getId().isEmpty())
    {
        block->addBlockErrorMsg(tr("Error: empty ID"));
    }

    //property errors -- cannot continue
    if (not cache->updateChangedProperties(block))
    {
        block->addBlockErrorMsg(tr("Error: cannot evaluate this block with property errors"));
        return Pothos::Proxy();
    }

    //evaluate the block and load its port info
    try
    {
        blockEval.callProxy("eval", block->getId().toStdString(), block->getBlockDesc());
        auto proxyBlock = blockEval.callProxy("getProxyBlock");
        cache->cacheInputPortInfo(proxyBlock.call<std::vector<Pothos::PortInfo>>("inputPortInfo"));
        cache->cacheOutputPortInfo(proxyBlock.call<std::vector<Pothos::PortInfo>>("outputPortInfo"));
    }

    //parser errors report
    catch(const Pothos::Exception &ex)
    {
        poco_error(Poco::Logger::get("PothosGui.TopologyEngine.evalGraphBlock"), ex.displayText());
        block->addBlockErrorMsg(QString::fromStdString(ex.message()));
    }

    //update the cache and return
    cache->loadCacheIntoBlock(block);
    cache->markUpdateDone();
    return cache->getProxyBlock();
}
