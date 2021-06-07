// Copyright (c) 2014-2021 Josh Blum
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

/***********************************************************************
 * msvc compiler wrapper
 **********************************************************************/
class MsvcCompilerSupport : public Pothos::Util::Compiler
{
public:

    MsvcCompilerSupport(void)
    {
        Poco::Path path("@MSVC_INSTALL_PATH@");

        //MSVC 2017 and up:
        //Look several directories up for the vcvarsall.bat script.
        //This script gets invoked with the architecture name,
        //which is assumed to match the name of the linker directory.
        #if _MSC_VER >= 1910 //2017 and up
        _vcvars_arch = path.getFileName();
        for (size_t i = 0; i < 8; i++)
        {
            if (path.getFileName() == "VC") break;
            path = path.makeParent().makeFile();
        }
        if (path.getFileName() != "VC") return;
        path = path.append("Auxiliary");
        path = path.append("Build");
        path = path.append("vcvarsall.bat");
        _vcvars_path = path.toString();

        //MSVC 2015 and below:
        //Look for the vcvarsxx.bat in the linker/tools executable directory.
        //This script is invoked without arguments to source the compiler tools.
        #else
        std::vector<std::string> files; Poco::File(path).list(files);
        for (const auto &file : files)
        {
            if (file.find("vcvars") != 0) continue; //expecting vcvarsxx.bat
            _vcvars_path = Poco::Path(path, file).absolute().toString();
        }

        #endif
    }

    bool test(void)
    {
        return not _vcvars_path.empty();
    }

    std::string compileCppModule(const Pothos::Util::CompilerArgs &args);

private:
    std::string _vcvars_path;
    std::string _vcvars_arch;
};

std::string MsvcCompilerSupport::compileCppModule(const Pothos::Util::CompilerArgs &compilerArgs)
{
    //create compiler bat script
    const auto clBatPath = this->createTempFile(".bat");
    std::ofstream clBatFile(clBatPath.c_str());
    clBatFile << "call \"" << _vcvars_path << "\"" << " " << _vcvars_arch << std::endl;
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
        args.push_back("/Tp"); //Specifies a C++ source file
        args.push_back("\""+source+"\"");
    }

    //specify object output
    const auto objPath = this->createTempFile(".obj");
    args.push_back("/Fo"+objPath); //required: no space between option and argument

    //create temp out file
    const auto outPath = this->createTempFile(Poco::SharedLibrary::suffix());
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

    //read into output buffer until pipe is closed
    Poco::PipeInputStream is(outPipe);
    std::string outBuff;
    for (std::string line; std::getline(is, line);) outBuff += line;

    //handle error case
    if (ph.wait() != 0 or not Poco::File(outPath.c_str()).exists())
    {
        throw Pothos::Exception("MsvcCompilerSupport::compileCppModule", outBuff);
    }

    //return output file path
    return outPath;
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
