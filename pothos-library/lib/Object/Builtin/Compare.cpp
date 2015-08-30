// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <string>

static int compareNullObjects(const Pothos::NullObject &, const Pothos::NullObject &)
{
    return 0; //always equal
}

static int compareStrings(const std::string &s0, const std::string &s1)
{
    return s0.compare(s1);
}

pothos_static_block(pothosObjectRegisterCompare)
{
    Pothos::PluginRegistry::addCall("/object/compare/null_to_null", &compareNullObjects);
    Pothos::PluginRegistry::addCall("/object/compare/string_to_string", &compareStrings);
}
