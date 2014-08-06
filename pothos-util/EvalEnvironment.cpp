// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "EvalEnvironment.hpp"
#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Util/EvalInterface.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Poco/TemporaryFile.h>
#include <Poco/ClassLoader.h>
#include <Poco/DigestStream.h>
#include <Poco/MD5Engine.h>
#include <Poco/File.h>
#include <Poco/NumberParser.h>
#include <Poco/RWLock.h>
#include <Poco/String.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

struct EvalEnvironment::EvalEnvironment::Impl
{
    ~Impl(void)
    {
        for (const auto &outPath : tmpModuleFiles)
        {
            try{loader.unloadLibrary(outPath);}catch(const Poco::Exception &){}
            try{Poco::File(outPath).remove();}catch(const Poco::Exception &){}
        }
    }
    Poco::ClassLoader<Pothos::Util::EvalInterface> loader;
    std::vector<std::string> tmpModuleFiles;
    std::map<std::string, Pothos::Object> evalCache;
    Poco::RWLock mutex;
};

EvalEnvironment::EvalEnvironment(void):
    _impl(new Impl())
{
    return;
}

Pothos::Object EvalEnvironment::eval(const std::string &expr_)
{
    const auto expr = Poco::trim(expr_);

    //check the cache
    {
        Poco::RWLock::ScopedReadLock l(_impl->mutex);
        auto it = _impl->evalCache.find(expr);
        if (it != _impl->evalCache.end()) return it->second;
    }

    auto result = this->evalNoCache(expr);

    //cache result and return
    {
        Poco::RWLock::ScopedWriteLock l(_impl->mutex);
        _impl->evalCache[expr] = result;
    }

    return result;
}

Pothos::Object EvalEnvironment::evalNoCache(const std::string &expr)
{
    if (expr.empty()) throw Pothos::Exception("EvalEnvironment::eval()", "expression is empty");

    //is it a string in quotes?
    if (std::count(expr.begin(), expr.end(), '"') == 2 and expr.front() == '"' and expr.back() == '"')
    {
        return Pothos::Object(expr.substr(1, expr.size()-2));
    }

    //list syntax mode
    if (expr.size() >= 2 and expr.front() == '[' and expr.back() == ']')
    {
        Pothos::ObjectVector vec;
        const auto noBrackets = expr.substr(1, expr.size()-2);
        for (const auto &tok : EvalEnvironment::splitExpr(noBrackets, ','))
        {
            try
            {
                vec.emplace_back(this->eval(tok));
            }
            catch (const Pothos::Exception &ex)
            {
                throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.message());
            }
        }
        return Pothos::Object(vec);
    }

    //map syntax mode
    if (expr.size() >= 2 and expr.front() == '{' and expr.back() == '}')
    {
        Pothos::ObjectMap map;
        const auto noBrackets = expr.substr(1, expr.size()-2);
        for (const auto &tok : EvalEnvironment::splitExpr(noBrackets, ','))
        {
            try
            {
                const auto keyVal = EvalEnvironment::splitExpr(tok, ':');
                if (keyVal.size() != 2) throw Pothos::Exception("EvalEnvironment::eval("+tok+")", "not key:value");
                map.emplace(this->eval(keyVal[0]), this->eval(keyVal[1]));
            }
            catch (const Pothos::Exception &ex)
            {
                throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.message());
            }
        }
        return Pothos::Object(map);
    }

    //support booleans
    if (expr == "true") return Pothos::Object(true);
    if (expr == "false") return Pothos::Object(false);

    //poco number parsers only work properly when expression has no spaces
    if (expr.find_first_of("\t\n ") == std::string::npos)
    {
        //try to parse regular unsigned, signed integers
        try {return Pothos::Object(Poco::NumberParser::parseUnsigned(expr));}
        catch (const Poco::SyntaxException &){}
        try {return Pothos::Object(Poco::NumberParser::parse(expr));}
        catch (const Poco::SyntaxException &){}

        //try to parse large unsigned, signed integers
        try {return Pothos::Object(Poco::NumberParser::parseUnsigned64(expr));}
        catch (const Poco::SyntaxException &){}
        try {return Pothos::Object(Poco::NumberParser::parse64(expr));}
        catch (const Poco::SyntaxException &){}

        //this hex parser does not require a 0x prefix -- dont want it
        //try {return Pothos::Object(Poco::NumberParser::parseHex64(expr));}
        //catch (const Poco::SyntaxException &){}

        try {return Pothos::Object(Poco::NumberParser::parseFloat(expr));}
        catch (const Poco::SyntaxException &){}
    }

    const auto compiler = Pothos::Util::Compiler::make();

    //get a hash of the expr so its symbol is unique
    Poco::MD5Engine md5; md5.update(expr);
    const auto symName = Poco::DigestEngine::digestToHex(md5.digest());

    //create the source to eval the string
    std::ostringstream oss;
    oss << "#define POCO_NO_AUTOMATIC_LIBS" << std::endl;
    oss << "#include <Pothos/Framework.hpp>" << std::endl;
    oss << "#include <Pothos/Util/EvalInterface.hpp>" << std::endl;
    oss << "#include <Poco/ClassLibrary.h>" << std::endl;
    oss << "using namespace Pothos;" << std::endl;
    oss << "struct Eval_" << symName << " : Pothos::Util::EvalInterface" << std::endl;
    oss << "{" << std::endl;
    oss << "Pothos::Object eval(void) const {return Object(" << expr << ");}" << std::endl;
    oss << "};" << std::endl;
    oss << "POCO_BEGIN_MANIFEST(Pothos::Util::EvalInterface)" << std::endl;
    oss << "    POCO_EXPORT_CLASS(Eval_" << symName << ")" << std::endl;
    oss << "POCO_END_MANIFEST" << std::endl;

    //perform compilation
    Pothos::Util::CompilerArgs args = Pothos::Util::CompilerArgs::defaultDevEnv();
    args.sources.push_back(oss.str());
    auto outMod = compiler->compileCppModule(args);

    //write module to file and load
    const auto outPath = Poco::TemporaryFile::tempName() + Poco::SharedLibrary::suffix();
    std::ofstream(outPath.c_str(), std::ios::binary).write(outMod.data(), outMod.size());
    _impl->tmpModuleFiles.push_back(outPath);
    Poco::ClassLoader<Pothos::Util::EvalInterface> loader;
    try
    {
        loader.loadLibrary(outPath);
    }
    catch (const Poco::Exception &ex)
    {
        throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.displayText());
    }

    //extract the symbol and call its evaluation routine
    std::shared_ptr<Pothos::Util::EvalInterface> eval0(loader.create("Eval_"+symName));
    Pothos::Object result = eval0->eval();
    return result;
}

#include <Pothos/Managed.hpp>

static auto managedEvalEnvironment = Pothos::ManagedClass()
    .registerConstructor<EvalEnvironment>()
    .registerMethod(POTHOS_FCN_TUPLE(EvalEnvironment, eval))
    .commit("Pothos/Util/EvalEnvironment");
