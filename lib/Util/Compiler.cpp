// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Callable.hpp>
#include <Pothos/Plugin.hpp>
#include <Pothos/System/Paths.hpp>
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/TemporaryFile.h>

Pothos::Util::CompilerArgs::CompilerArgs(void)
{
    return;
}

Pothos::Util::CompilerArgs Pothos::Util::CompilerArgs::defaultDevEnv(void)
{
    Pothos::Util::CompilerArgs args;

    //add devel environment libraries
    Poco::Path path(Pothos::System::getPothosDevLibraryPath());
    std::vector<std::string> files; Poco::File(path).list(files);
    for (size_t i = 0; i < files.size(); i++)
    {
        Poco::Path library(path, files[i]);
        if ((
            library.getExtension() == "so" or
            library.getExtension() == "lib" or
            library.getExtension() == "dylib"
        ) and
        (
            files[i].find("Poco") != std::string::npos or
            files[i].find("Pothos") != std::string::npos
        )) args.libraries.push_back(library.absolute().toString());
    }

    //add devel environment includes
    args.includes.push_back(Pothos::System::getPothosDevIncludePath());

    return args;
}

Pothos::Util::Compiler::~Compiler(void)
{
    for (const auto &path : _tempFiles)
    {
        Poco::File f(path);
        if (f.exists()) try
        {
            f.remove();
        }
        catch(...){}
    }
}

Pothos::Util::Compiler::Sptr Pothos::Util::Compiler::make(const std::string &name)
{
    //handle the empty name/automatic case
    if (name.empty())
    {
        for (const auto &factory_name : PluginRegistry::list("/util/compiler"))
        {
            auto compiler = Pothos::Util::Compiler::make(factory_name);
            if (compiler->test()) return compiler;
        }
        throw Pothos::Exception("Pothos::Util::Compiler::make()", "no available or passing factories");
    }

    //locate the compiler factory and make it
    Sptr compiler;
    try
    {
        auto plugin = Pothos::PluginRegistry::get(Pothos::PluginPath("/util/compiler").join(name));
        auto callable = plugin.getObject().extract<Pothos::Callable>();
        compiler = callable.call<Sptr>();
    }
    catch(const Exception &ex)
    {
        throw Pothos::Exception("Pothos::Util::Compiler::make("+name+")", ex);
    }
    return compiler;
}

const std::string &Pothos::Util::Compiler::createTempFile(const std::string &ext)
{
    _tempFiles.push_back(Poco::TemporaryFile::tempName() + ext);
    return _tempFiles.back();
}
