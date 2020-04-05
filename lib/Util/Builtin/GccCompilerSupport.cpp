// Copyright (c) 2014-2016 Josh Blum
//                    2020 Nicholas Corgan
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

    //add compiler flag for C++ standard used to compile Pothos
#if __cplusplus >= 201704L
    // DTODO: this may change down the line, so this may need to be smarter
    args.push_back("-std=c++2a");
#elif __cplusplus >= 201703L
    args.push_back("-std=c++17");
#elif __cplusplus >= 201702L
    args.push_back("-std=c++14");
#else
    args.push_back("-std=c++11");
#endif

    //add compiler flags
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

    //handle error case
    if (ph.wait() != 0)
    {
        Poco::PipeInputStream errStream(outPipe);
        const std::string errMsgBuff = std::string(
            std::istreambuf_iterator<char>(errStream),
            std::istreambuf_iterator<char>());
        throw Pothos::Exception("GccCompilerSupport::compileCppModule", errMsgBuff);
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
