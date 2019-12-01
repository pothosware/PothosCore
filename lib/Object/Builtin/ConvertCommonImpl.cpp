// Copyright (c) 2019 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "ConvertCommonImpl.hpp"

void registerConvertNum(const std::string &inName, const std::string &outName, const Pothos::Callable &callable)
{
    const std::string name = inName + "_to_" + outName;
    auto path = Pothos::PluginPath("/object/convert/numbers").join(name);
    Pothos::PluginRegistry::add(path, callable);
}

void registerConvertVec(const std::string &inName, const std::string &outName, const Pothos::Callable &callable)
{
    const std::string name = inName + "_to_" + outName;
    auto path = Pothos::PluginPath("/object/convert/vectors").join(name);
    Pothos::PluginRegistry::add(path, callable);
}
