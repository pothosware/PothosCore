// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Exception.hpp>
#include <Pothos/Managed.hpp>
#include <Pothos/Plugin.hpp>

#include "json.hpp"

#include <Poco/Format.h>

#include <algorithm>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

/*
Example (all fields required):

{
    {"namespace": "PothosBlocksSIMD"},
    {
        "functions":
        [
            {
                {"name": "clamp"},
                {"returnType": "void"},
                {"paramTypes": ["T"]},
                {"params": ["const T*", "const T*", "const T&", "const T&", "size_t"]}
            },
            {
                {"name": "minmax"},
                {"returnType": "void"},
                {"paramTypes": ["T"]},
                {"params": ["const T**", "T*", "T*", "size_t", "size_t"]}
            }
        ]
    }
}

*/

//
// Common
//

struct SIMDInfo
{
    // From input parameters
    std::string simdNamespace;
    std::vector<std::string> arches;

    // From JSON
    std::string name;
    std::string returnType;
    std::vector<std::string> paramTypes;
    std::vector<std::string> params;
};

//
// String generation
//

static std::string join(const std::vector<std::string>& stringVec, const std::string& delimiter)
{
    std::stringstream sstream;
    for (const auto& str : stringVec)
    {
        if (&str != &stringVec[0]) sstream << delimiter;
        sstream << str;
    }

    return sstream.str();
}

static inline std::string getParamTypeString(const SIMDInfo& simdInfo)
{
    return join(simdInfo.paramTypes, ",");
}

static inline std::string getTemplateString(const SIMDInfo& simdInfo)
{
    return Poco::format("template <%s>", getParamTypeString(simdInfo));
}

static inline std::string getParamString(const SIMDInfo& simdInfo)
{
    return join(simdInfo.params, ",");
}

static inline std::string getDeclTypeString(const SIMDInfo& simdInfo)
{
    return Poco::format("decltype(&%s::fallback::%s<%s>)",
               simdInfo.simdNamespace,
               simdInfo.name,
               getParamTypeString(simdInfo));
}

static inline std::string getUsingFunctionTypeString(const SIMDInfo& simdInfo)
{
    return Poco::format("%s using %sFcn = %s<%s>;",
               getTemplateString(simdInfo),
               simdInfo.name,
               getDeclTypeString(simdInfo),
               getParamTypeString(simdInfo));
}

static inline std::string getArchFuncDeclString(const SIMDInfo& simdInfo, const std::string& arch)
{
    return Poco::format(
               "namespace %s { namespace %s { %s %s %s(%s); } }",
               simdInfo.simdNamespace,
               arch,
               getTemplateString(simdInfo),
               simdInfo.returnType,
               getParamString(simdInfo));
}

static std::string getFuncDeclString(const SIMDInfo& simdInfo)
{
    std::vector<std::string> funcDeclVec;
    funcDeclVec.reserve(simdInfo.arches.size());

    std::transform(
        simdInfo.arches.begin(),
        simdInfo.arches.end(),
        std::back_inserter(funcDeclVec),
        std::bind(
            &getArchFuncDeclString,
            simdInfo,
            std::placeholders::_1));

    return join(funcDeclVec, "\n");
}

static std::string getMapEntry(
    const SIMDInfo& simdInfo,
    const std::string& arch)
{
    return Poco::format(
               "{\"%s::%s::%s\"}, &%s::%s::%s",
               simdInfo.simdNamespace,
               arch,
               simdInfo.name,
               simdInfo.simdNamespace,
               arch,
               simdInfo.name);
}

static std::string getMapEntries(const SIMDInfo& simdInfo)
{
    std::vector<std::string> mapEntryVec;
    mapEntryVec.reserve(simdInfo.arches.size());

    std::transform(
        simdInfo.arches.begin(),
        simdInfo.arches.end(),
        std::back_inserter(mapEntryVec),
        std::bind(
            &getMapEntry,
            simdInfo,
            std::placeholders::_1));

    return join(mapEntryVec, ",");
}

static std::string getMapString(const SIMDInfo& simdInfo)
{
    return Poco::format(
               "static const std::unordered_map<std::string, %s> Impls = {%s};",
               getDeclTypeString(simdInfo),
               getMapEntries(simdInfo));
}

static inline std::string getFeatureSetKeyString(const SIMDInfo& simdInfo)
{
    return Poco::format("Pothos::System::getOptionalSIMDFeatureSetKey({%s})", join(simdInfo.arches, ","));
}

static std::string getFullFuncString(const SIMDInfo& simdInfo)
{
    static const std::string formatStr =
        "%s"
        "namespace %s"
        "{"
        "%s"
        "    static %s %sDispatch()"
        "    {"
        "%s"
        "auto implIter = Impls.find(%s);"
        "if (implIter != implIter.end()) return implIter->second;"
        "else return Impls.at(\"fallback\");"
        "    }"
        "}";

    return Poco::format(
               formatStr,
               getFuncDeclString(simdInfo),
               simdInfo.simdNamespace,
               getUsingFunctionTypeString(simdInfo),
               getDeclTypeString(simdInfo),
               simdInfo.name,
               getFeatureSetKeyString(simdInfo));
}

std::string getAllFuncsString(const std::vector<SIMDInfo>& allSIMDInfo)
{
    std::vector<std::string> allFuncsVec;
    allFuncsVec.reserve(allSIMDInfo.size());

    std::transform(
        allSIMDInfo.begin(),
        allSIMDInfo.end(),
        std::back_inserter(allFuncsVec),
        &getFullFuncString);

    return join(allFuncsVec, "\n\n");
}

//
// JSON importing
//

// Namespace separate due to being on higher layer
static SIMDInfo jsonToSIMDInfo(
    const nlohmann::json& json,
    const std::string& simdNamespace,
    const std::vector<std::string>& arches)
{
    SIMDInfo simdInfo;
    simdInfo.simdNamespace = simdNamespace;
    simdInfo.arches = arches;
    simdInfo.name = json["name"].get<std::string>();
    simdInfo.returnType = json["returnType"].get<std::string>();
    simdInfo.paramTypes = json["paramTypes"].get<std::vector<std::string>>();
    simdInfo.params = json["params"].get<std::vector<std::string>>();

    return simdInfo;
}

static std::vector<SIMDInfo> importSIMDFromJSON(
    const nlohmann::json& json,
    const std::vector<std::string>& arches)
{
    const auto simdNamespace = json["namespace"].get<std::string>();
    const auto fcnArray = json["functions"];

    std::vector<SIMDInfo> allSIMDInfo;
    std::transform(
        fcnArray.begin(),
        fcnArray.end(),
        std::back_inserter(allSIMDInfo),
        std::bind(
            &jsonToSIMDInfo,
            std::placeholders::_1,
            simdNamespace, arches));

    return allSIMDInfo;
}

//
//
//

static inline std::string getSIMDDispatcherString(
    const nlohmann::json& json,
    const std::vector<std::string>& arches)
{
    return getAllFuncsString(importSIMDFromJSON(json, arches));
}

class SIMDDispatcherUtils
{
public:
    static std::string generateAndDumpHeader(
        const std::string& jsonString,
        const std::vector<std::string>& arches)
    {
        auto json = nlohmann::json::parse(jsonString);

        return getAllFuncsString(importSIMDFromJSON(json, arches));
    }
};

static auto managedDocUtils = Pothos::ManagedClass()
    .registerClass<SIMDDispatcherUtils>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(SIMDDispatcherUtils, generateAndDumpHeader))
    .commit("Pothos/Util/SIMDDispatcher");