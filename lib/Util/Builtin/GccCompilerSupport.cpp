// Copyright (c) 2014-2021 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/Process.h>
#include <Poco/TemporaryFile.h>
#include <Poco/SharedLibrary.h>
#include <fstream>
#include <iostream>

/***********************************************************************
 * gcc compiler wrapper
 **********************************************************************/
class GccCompilerSupport : public Pothos::Util::Compiler
{
public:

    GccCompilerSupport(void)
    {
        return;
    }

    bool test(void)
    {
        Poco::Process::Args args;
        args.push_back("--version");
        Poco::Process::Env env;
        Poco::Pipe outPipe;
        Poco::ProcessHandle ph(Poco::Process::launch(
            "g++", args, nullptr, &outPipe, &outPipe, env));
        return ph.wait() == 0;
    }

    std::string compileCppModule(const Pothos::Util::CompilerArgs &args);
};

std::string GccCompilerSupport::compileCppModule(const Pothos::Util::CompilerArgs &compilerArgs)
{
    //create args
    Poco::Process::Args args;

    //add libraries
    for (const auto &library : compilerArgs.libraries)
    {
        args.push_back(library);
    }

    //add compiler flags
    args.push_back("-std=c++11");
    args.push_back("-shared");
    args.push_back("-fPIC");
    for (const auto &flag : compilerArgs.flags)
    {
        args.push_back(flag);
    }

    //add include paths
    for (const auto &include : compilerArgs.includes)
    {
        args.push_back("-I");
        args.push_back(include);
    }

    //add compiler sources
    args.push_back("-x");
    args.push_back("c++");
    for (const auto &source : compilerArgs.sources)
    {
        args.push_back(source);
    }

    //create temp out file
    const auto outPath = this->createTempFile(Poco::SharedLibrary::suffix());
    args.push_back("-o");
    args.push_back(outPath);

    //launch
    Poco::Pipe inPipe, outPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        "g++", args, &inPipe, &outPipe, &outPipe, env));

    //read into output buffer until pipe is closed
    Poco::PipeInputStream is(outPipe);
    std::string outBuff;
    for (std::string line; std::getline(is, line);) outBuff += line;

    //handle error case
    if (ph.wait() != 0 or not Poco::File(outPath.c_str()).exists())
    {
        throw Pothos::Exception("GccCompilerSupport::compileCppModule", outBuff);
    }

    //return output file path
    return outPath;
}

/***********************************************************************
 * factory and registration
 **********************************************************************/
Pothos::Util::Compiler::Sptr makeGccCompilerSupport(void)
{
    return Pothos::Util::Compiler::Sptr(new GccCompilerSupport());
}

pothos_static_block(pothosUtilRegisterGccCompilerSupport)
{
    Pothos::PluginRegistry::addCall("/util/compiler/gcc", &makeGccCompilerSupport);
}
