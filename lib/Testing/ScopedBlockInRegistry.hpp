// Copyright (c) 2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <Pothos/Callable.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Plugin.hpp>

#include <Poco/Exception.h>

#include <iostream>
#include <memory>
#include <string>

struct ScopedBlockInRegistry
{
    ScopedBlockInRegistry(
        const std::string& blockPath,
        const Pothos::Callable& callable
    ):
        blockPath(blockPath),
        blockRegistryUPtr(new Pothos::BlockRegistry(blockPath, callable))
    {
    }

    virtual ~ScopedBlockInRegistry()
    {
        POTHOS_EXCEPTION_TRY
        {
            const std::string fullBlockPath = "/blocks" + blockPath;
            Pothos::PluginRegistry::remove(fullBlockPath);
        }
        POTHOS_EXCEPTION_CATCH(const Pothos::Exception& ex)
        {
            std::cerr << ex.what() << std::endl;            
        }
    }

    std::string blockPath;
    std::unique_ptr<Pothos::BlockRegistry> blockRegistryUPtr;
};
