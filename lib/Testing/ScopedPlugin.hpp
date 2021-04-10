// Copyright (c) 2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once

#include <Pothos/Exception.hpp>
#include <Pothos/Plugin.hpp>

#include <Poco/Exception.h>

#include <iostream>
#include <string>

template <typename ValueType>
struct ScopedPlugin
{
    ScopedPlugin(
        const std::string& path,
        ValueType&& value
    ):
        path(path)
    {
        Pothos::PluginRegistry::add(path, std::forward<ValueType>(value));
    }

    virtual ~ScopedPlugin()
    {
        POTHOS_EXCEPTION_TRY
        {
            Pothos::PluginRegistry::remove(path);
        }
        POTHOS_EXCEPTION_CATCH(const Pothos::Exception& ex)
        {
            std::cerr << ex.what() << std::endl;            
        }
    }

    std::string path;
};
