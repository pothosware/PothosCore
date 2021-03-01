// Copyright (c) 2020-2021 Nicholas Corgan
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
Example (all fields required except addedIncludes):

{
    "namespace": "Test",
    "functions":
    [
        {
            "name": "clamp",
            "returnType": "void",
            "params": ["const float*", "float*", "float", "float", "size_t"]
        },
        {
            "name": "minmax",
            "returnType": "void",
            "paramTypes": ["T"],
            "params": ["const T**", "T*", "T*", "size_t", "size_t"]
        },
        {
            "name": "vectorConvert",
            "returnType": "std::vector<OutType>",
            "paramTypes": ["InType", "OutType"],
            "params": ["const std::vector<InType>&"]
            "addedIncludes": ["vector"]
        }
    ]
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
    std::vector<std::string> addedIncludes;
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

static std::string getAddedIncludesString(const SIMDInfo& simdInfo)
{
    std::vector<std::string> includeStrings;
    includeStrings.reserve(simdInfo.addedIncludes.size());

    std::transform(
        simdInfo.addedIncludes.begin(),
        simdInfo.addedIncludes.end(),
        std::back_inserter(includeStrings),
        [](const std::string& addedInclude)
        {
            return Poco::format("#include <%s>", addedInclude);
        });

    return join(includeStrings, "\n");
}

static inline std::string getParamTypeString(const SIMDInfo& simdInfo)
{
    return join(simdInfo.paramTypes, ", ");
}

// Accounts for whether function is templated or not
static std::string getFunctionString(const SIMDInfo& simdInfo)
{
    if(!simdInfo.paramTypes.empty())
    {
        return Poco::format(
                   "%s<%s>",
                   simdInfo.name,
                   getParamTypeString(simdInfo));
    }
    else return simdInfo.name;
}

// Accounts for whether type is templated or not
static std::string getFunctionTypeString(const SIMDInfo& simdInfo)
{
    if(!simdInfo.paramTypes.empty())
    {
        return Poco::format(
                   "%sFcn<%s>",
                   simdInfo.name,
                   getParamTypeString(simdInfo));
    }
    else return simdInfo.name+"Fcn";
}

static std::string getTemplateTypenameString(const SIMDInfo& simdInfo)
{
    if(simdInfo.paramTypes.empty()) return "";

    std::vector<std::string> typenameStrings;
    typenameStrings.reserve(simdInfo.paramTypes.size());

    std::transform(
        simdInfo.paramTypes.begin(),
        simdInfo.paramTypes.end(),
        std::back_inserter(typenameStrings),
        [](const std::string& paramType) {return Poco::format("typename %s", paramType); });

    return Poco::format("template <%s>", join(typenameStrings, ", "));
}

static inline std::string getParamString(const SIMDInfo& simdInfo)
{
    return join(simdInfo.params, ", ");
}

static inline std::string getDeclTypeString(const SIMDInfo& simdInfo)
{
    return Poco::format("decltype(&%s::fallback::%s)",
               simdInfo.simdNamespace,
               getFunctionString(simdInfo));
}

static inline std::string getUsingFunctionTypeString(const SIMDInfo& simdInfo)
{
    return Poco::format("%s using %sFcn = %s;",
               getTemplateTypenameString(simdInfo),
               simdInfo.name,
               getDeclTypeString(simdInfo));
}

static inline std::string getArchFuncDeclString(const SIMDInfo& simdInfo, const std::string& arch)
{
    return Poco::format(
               "namespace %s { %s %s %s(%s); }",
               arch,
               getTemplateTypenameString(simdInfo),
               simdInfo.returnType,
               simdInfo.name,
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
               "{\"%s\", &%s::%s::%s}",
               arch,
               simdInfo.simdNamespace,
               arch,
               getFunctionString(simdInfo));
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

    return join(mapEntryVec, ",\n");
}

static std::string getMapString(const SIMDInfo& simdInfo)
{
    return Poco::format(
               "static const std::unordered_map<std::string, %s> Impls = \n{\n%s\n};",
               getFunctionTypeString(simdInfo),
               getMapEntries(simdInfo));
}

static std::string getFeatureSetKeyString(const SIMDInfo& simdInfo)
{
    std::vector<std::string> archesWithQuotes;
    archesWithQuotes.reserve(simdInfo.arches.size());

    std::transform(
        simdInfo.arches.begin(),
        simdInfo.arches.end(),
        std::back_inserter(archesWithQuotes),
        [](const std::string& arch)
        {
            return "\"" + arch + "\"";
        });

    return Poco::format("Pothos::System::getOptimalSIMDFeatureSetKey(std::vector<std::string>{%s})", join(archesWithQuotes, ","));
}

static std::string getFullFuncString(const SIMDInfo& simdInfo)
{
    static const std::string formatStr =
        "%s\n\n"

        "namespace %s\n"
        "{\n\n"

        "%s\n\n"

        "    %s\n\n"

        "    %s\n"
        "    %s %sDispatch()\n"
        "    {\n"
        "%s\n"
        "        auto implIter = Impls.find(%s);\n"
        "        if (implIter != Impls.end()) return implIter->second;\n"
        "        else return Impls.at(\"fallback\");\n"
        "    }\n"
        "}";

    return Poco::format(
               formatStr,

               getAddedIncludesString(simdInfo),
               simdInfo.simdNamespace,
               getFuncDeclString(simdInfo),
               getUsingFunctionTypeString(simdInfo),
               getTemplateTypenameString(simdInfo),
               getFunctionTypeString(simdInfo),
               simdInfo.name,
               getMapString(simdInfo),
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

    return join(allFuncsVec, "\n\n/**************************************/\n\n");
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
    simdInfo.params = json["params"].get<std::vector<std::string>>();

    auto paramTypesIter = json.find("paramTypes");
    if (paramTypesIter != json.end()) simdInfo.paramTypes = paramTypesIter->get<std::vector<std::string>>();

    auto addedIncludesIter = json.find("addedIncludes");
    if (addedIncludesIter != json.end()) simdInfo.addedIncludes = addedIncludesIter->get<std::vector<std::string>>();

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
// Caller-facing
//

static inline std::string getSIMDDispatcherString(
    const nlohmann::json& json,
    const std::vector<std::string>& arches)
{
    static const std::string formatStr =
        "// Machine-generated code\n\n"

        "#pragma once\n\n"

        "#include <Pothos/System/SIMD.hpp>\n"
        "#include <string>\n"
        "#include <unordered_map>\n"
        "#include <vector>\n\n"

        "%s";

    return Poco::format(formatStr, getAllFuncsString(importSIMDFromJSON(json, arches)));
}

class SIMDDispatcherUtils
{
public:
    static std::string generateAndDumpHeader(
        const std::string& jsonString,
        const std::vector<std::string>& arches)
    {
        // Add fallback here so it's in the output map
        std::vector<std::string> archesWithFallback(arches);
        archesWithFallback.emplace_back("fallback");

        return getSIMDDispatcherString(nlohmann::json::parse(jsonString), archesWithFallback);
    }
};

static auto managedDocUtils = Pothos::ManagedClass()
    .registerClass<SIMDDispatcherUtils>()
    .registerStaticMethod(POTHOS_FCN_TUPLE(SIMDDispatcherUtils, generateAndDumpHeader))
    .commit("Pothos/Util/SIMDDispatcher");
