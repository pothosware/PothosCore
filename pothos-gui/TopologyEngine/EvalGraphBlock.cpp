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
    POTHOS_EXCEPTION_TRY
    {
        env = this->getEnvironmentFromZone(block->getAffinityZone());
        evalEnv = this->getEvalEnvironment(block->getAffinityZone());
    }
    POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
    {
        block->addBlockErrorMsg(QString::fromStdString(ex.displayText()));
        return Pothos::Proxy();
    }

    //grab the cache
    auto &cache = _idToBlockEval[block->getId()];

    //blank id is for a temporary
    if (block->getId().isEmpty()) cache.reset();

    //clear the cache if irrelevant
    if (cache and cache->getProxyBlock())
    {
        //the block moved to another environment
        auto oldEnv = cache->getProxyBlock().getEnvironment();
        if (oldEnv->getUniquePid() != env->getUniquePid()) cache.reset();
    }

    //create a new cache
    const bool newCache = not cache;
    if (not cache) cache.reset(new EvalBlockCache(block->getBlockDesc()));

    //load all the current properties into the cache
    for (const auto &propKey : block->getProperties())
    {
        cache->loadCurrentValue(propKey, block->getPropertyValue(propKey));
    }

    //deal with non-critical changes -- must have a valid block to apply settings to
    if (not newCache and not cache->hasCriticalChange() and cache->getProxyBlock().callProxy("getProxyBlock"))
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
