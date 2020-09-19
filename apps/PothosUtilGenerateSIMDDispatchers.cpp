// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Pothos/Util/BlockDescription.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Format.h>
#include <Poco/NumberFormatter.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

void PothosUtilBase::generateSIMDDispatchers(const std::string&, const std::string& jsonPath)
{

}

/***********************************************************************
 * generate a cpp source that adds the json string to the registry
 **********************************************************************/
/*
static void jsonArrayToCppStaticBlock(const Pothos::Util::BlockDescriptionParser &parser, std::ostream &os, const std::string &target)
{
    os << "#include <Pothos/Framework.hpp>\n";
    os << "#include <Pothos/Plugin.hpp>\n";
    os << Poco::format("pothos_static_block(registerPothosBlockDocs%s)\n", target);
    os << "{\n";
    for (const auto &factory : parser.listFactories())
    {
        //create escaped string of json
        std::string escaped;
        for (const auto &ch : parser.getJSONObject(factory))
        {
            escaped += "\\x" + Poco::NumberFormatter::formatHex(int(ch), 2width, falseno 0x);
        }

        //register the block description at the specified path
        const auto pluginPath = Pothos::PluginPath("/blocks/docs", factory);
        os << Poco::format("    Pothos::PluginRegistry::add(\"%s\", std::string(\"%s\"));\n", pluginPath.toString(), escaped);
    }
    os << "}\n";
}

void PothosUtilBase::docParse(const std::vector<std::string> &inputFilePaths)
{
    Pothos::Util::BlockDescriptionParser parser;

    //feed all input files into the parser
    for (const auto &inputFilePath : inputFilePaths)
    {
        parser.feedFilePath(inputFilePath);
    }

    //write to output (file if specified, otherwise stdout)
    const auto outputFilePath = this->config().getString("outputFile", "");
    const size_t indentSpaces = 4;
    if (outputFilePath.empty())
    {
        std::cout << std::endl;
        std::cout << parser.getJSONArray(indentSpaces);
        std::cout << std::endl;
    }
    else
    {
        const auto outputFileName = Poco::Path(outputFilePath).getBaseName();
        const auto outputFileExt = Poco::Path(outputFilePath).getExtension();

        std::ofstream outputFile(outputFilePath.c_str());
        if (outputFileExt == "json") outputFile << parser.getJSONArray(indentSpaces);
        else if (outputFileExt == "cpp") jsonArrayToCppStaticBlock(parser, outputFile, outputFileName);
        else throw Pothos::Exception("PothosUtilBase::docParse()", "unsupported file extension: " + outputFilePath);
        outputFile << std::endl;
        outputFile.close();
    }
}
*/