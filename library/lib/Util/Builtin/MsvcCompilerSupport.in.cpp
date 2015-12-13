// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/Pipe.h>
#include <Poco/PipeStream.h>
#include <Poco/Process.h>
#include <Poco/TemporaryFile.h>
#include <Poco/SharedLibrary.h>
#include <Poco/Logger.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <fstream>
#include <iostream>

static void cleanupTempFiles(const std::vector<std::string> &tempFilesToCleanup)
{
    for (const auto &path : tempFilesToCleanup)
    {
        Poco::File f(path);
        if (f.exists()) try
        {
            f.remove();
        }
        catch(...){}
    }
}

/***********************************************************************
 * gcc compiler wrapper
 **********************************************************************/
class MsvcCompilerSupport : public Pothos::Util::Compiler
{
public:

    MsvcCompilerSupport(void)
    {
        Poco::Path path("@MSVC_INSTALL_PATH@");
        std::vector<std::string> files; Poco::File(path).list(files);
        for (size_t i = 0; i < files.size(); i++)
        {
            if (files[i].find("vcvars") != 0) continue; //expecting vcvarsxx.bat
            _vcvars_path = Poco::Path(path, files[i]).absolute().toString();
        }
    }

    bool test(void)
    {
        return not _vcvars_path.empty();
    }

    std::string compileCppModule(const Pothos::Util::CompilerArgs &args);

private:
    std::string _vcvars_path;
};

std::string MsvcCompilerSupport::compileCppModule(const Pothos::Util::CompilerArgs &compilerArgs)
{
    std::vector<std::string> tempFilesToCleanup;

    //create compiler bat script
    const auto clBatPath = Poco::TemporaryFile::tempName() + ".bat";
    tempFilesToCleanup.push_back(clBatPath);
    std::ofstream clBatFile(clBatPath.c_str());
    clBatFile << "call \"" << _vcvars_path << "\"" << std::endl;
    clBatFile << "cl.exe %*" << std::endl;
    clBatFile << "exit /b %ERRORLEVEL%" << std::endl;
    clBatFile.close();

    //create args
    Poco::Process::Args args;
    args.push_back("/LD"); //Creates a dynamic-link library
    #ifdef _DEBUG
    args.push_back("/MDd"); //Creates a multithreaded DLL
    #else
    args.push_back("/MD"); //Creates a multithreaded DLL
    #endif

    //add libraries
    for (const auto &library : compilerArgs.libraries)
    {
        args.push_back("\""+library+"\"");
    }

    //add compiler flags
    for (const auto &flag : compilerArgs.flags)
    {
        args.push_back(flag);
    }

    //add include paths
    for (const auto &include : compilerArgs.includes)
    {
        args.push_back("/I");
        args.push_back("\""+include+"\"");
    }

    //add compiler sources
    for (const auto &source : compilerArgs.sources)
    {
        const auto filePath = Poco::TemporaryFile::tempName() + ".cpp";
        tempFilesToCleanup.push_back(filePath);
        std::ofstream(filePath.c_str()).write(source.data(), source.size());
        args.push_back("/Tp"); //Specifies a C++ source file
        args.push_back(filePath);
    }

    //specify object output
    const auto objPath = Poco::TemporaryFile::tempName() + ".obj";
    tempFilesToCleanup.push_back(objPath);
    args.push_back("/Fo"+objPath); //required: no space between option and argument

    //create temp out file
    const auto outPath = Poco::TemporaryFile::tempName() + Poco::SharedLibrary::suffix();
    tempFilesToCleanup.push_back(outPath);
    args.push_back("/link");
    args.push_back("/out:"+outPath);

    //log the command
    std::string cmdToLog = "cl.exe ";
    for (const auto &a : args) cmdToLog += a + " ";
    //poco_information(Poco::Logger::get("Pothos.MsvcCompilerSupport.compileCppModule"), cmdToLog);

    //launch
    Poco::Pipe outPipe;
    Poco::Process::Env env;
    Poco::ProcessHandle ph(Poco::Process::launch(
        clBatPath, args, nullptr, &outPipe, &outPipe, env));

    //handle error case
    if (ph.wait() != 0 or not Poco::File(outPath.c_str()).exists())
    {
        Poco::PipeInputStream errStream(outPipe);
        const std::string errMsgBuff = std::string(
            std::istreambuf_iterator<char>(errStream),
            std::istreambuf_iterator<char>());
        cleanupTempFiles(tempFilesToCleanup);
        throw Pothos::Exception("MsvcCompilerSupport::compileCppModule", errMsgBuff);
    }

    //output file to string
    std::ifstream outFile(outPath.c_str(), std::ios::binary);
    const std::string outBuff = std::string(
        std::istreambuf_iterator<char>(outFile),
        std::istreambuf_iterator<char>());
    cleanupTempFiles(tempFilesToCleanup);
    return outBuff;
}

/***********************************************************************
 * factory and registration
 **********************************************************************/
Pothos::Util::Compiler::Sptr makeMsvcCompilerSupport(void)
{
    return Pothos::Util::Compiler::Sptr(new MsvcCompilerSupport());
}

pothos_static_block(pothosUtilRegisterMsvcCompilerSupport)
{
    Pothos::PluginRegistry::addCall("/util/compiler/msvc", &makeMsvcCompilerSupport);
}
