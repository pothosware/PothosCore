// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "EvalEnvironment.hpp"
#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Util/EvalInterface.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Proxy.hpp>
#include <Poco/TemporaryFile.h>
#include <Poco/ClassLoader.h>
#include <Poco/DigestStream.h>
#include <Poco/MD5Engine.h>
#include <Poco/File.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Types.h>
#include <Poco/RWLock.h>
#include <Poco/String.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <future>

struct EvalEnvironment::Impl
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
    std::map<std::string, std::string> errorCache;
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

        //errors cache
        auto errorIt = _impl->errorCache.find(expr);
        if (errorIt != _impl->errorCache.end()) throw Pothos::Exception("EvalEnvironment::eval("+expr+")", errorIt->second);
    }

    //try to perform the evaluation
    Pothos::Object result;
    try
    {
        result = this->evalNoCache(expr);
    }
    catch (const Pothos::Exception &ex)
    {
        //cache the error
        Poco::RWLock::ScopedWriteLock l(_impl->mutex);
        _impl->errorCache[expr] = ex.displayText();
        throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.displayText());
    }

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
    const auto inQuotes = expr.size() >= 2 and expr.front() == '"' and expr.back() == '"';
    const auto inBrackets = expr.size() >= 2 and expr.front() == '[' and expr.back() == ']';
    const auto inBraces = expr.size() >= 2 and expr.front() == '{' and expr.back() == '}';

    //list syntax mode
    if (inBrackets)
    {
        std::vector<std::shared_future<Pothos::Object>> futures;
        const auto noBrackets = expr.substr(1, expr.size()-2);
        for (const auto &tok : EvalEnvironment::splitExpr(noBrackets, ','))
        {
            futures.push_back(std::async(std::launch::async, std::bind(&EvalEnvironment::eval, this, tok)));
        }

        auto env = Pothos::ProxyEnvironment::make("managed");
        Pothos::ProxyVector vec;
        for (const auto &future : futures)
        {
            try
            {
                vec.emplace_back(env->convertObjectToProxy(future.get()));
            }
            catch (const Pothos::Exception &ex)
            {
                throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.message());
            }
        }
        return Pothos::Object(vec);
    }

    //map syntax mode
    if (inBraces)
    {
        std::vector<std::shared_future<Pothos::Object>> futuresK, futuresV;
        const auto noBrackets = expr.substr(1, expr.size()-2);
        for (const auto &tok : EvalEnvironment::splitExpr(noBrackets, ','))
        {
            const auto keyVal = EvalEnvironment::splitExpr(tok, ':');
            if (keyVal.size() != 2) throw Pothos::Exception("EvalEnvironment::eval("+tok+")", "not key:value");
            futuresK.push_back(std::async(std::launch::async, std::bind(&EvalEnvironment::eval, this, keyVal[0])));
            futuresV.push_back(std::async(std::launch::async, std::bind(&EvalEnvironment::eval, this, keyVal[1])));
        }

        auto env = Pothos::ProxyEnvironment::make("managed");
        Pothos::ProxyMap map;
        for (size_t i = 0; i < futuresK.size(); i++)
        {
            try
            {
                const auto key = env->convertObjectToProxy(futuresK[i].get());
                const auto val = env->convertObjectToProxy(futuresV[i].get());
                map.emplace(key, val);
            }
            catch (const Pothos::Exception &ex)
            {
                throw Pothos::Exception("EvalEnvironment::eval("+expr+")", ex.message());
            }
        }
        return Pothos::Object(map);
    }

    //support simple numbers and booleans with JSON parser
    if (expr.find_first_of("\t\n ") == std::string::npos or inQuotes)
    {
        try
        {
            Poco::JSON::Parser p; p.parse("["+expr+"]");
            const auto val = p.getHandler()->asVar().extract<Poco::JSON::Array::Ptr>()->get(0);
            if (val.type() == typeid(bool)) return Pothos::Object(val.convert<bool>());
            if (val.type() == typeid(std::string)) return Pothos::Object(val.convert<std::string>());
            if (val.isNumeric() and val.isInteger())
            {
                try {return Pothos::Object(val.convert<Poco::UInt32>());} catch (const Poco::Exception &){}
                try {return Pothos::Object(val.convert<Poco::Int32>());} catch (const Poco::Exception &){}
                try {return Pothos::Object(val.convert<Poco::UInt64>());} catch (const Poco::Exception &){}
                try {return Pothos::Object(val.convert<Poco::Int64>());} catch (const Poco::Exception &){}
            }
            else if (val.isNumeric())
            {
                try {return Pothos::Object(val.convert<double>());} catch (const Poco::Exception &){}
            }
        }
        catch (const Poco::Exception &){}
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
    oss << "#include <complex>" << std::endl;
    oss << "#include <cmath>" << std::endl;
    oss << "static const auto j = std::complex<double>(0, 1);" << std::endl;
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
    .registerStaticMethod(POTHOS_FCN_TUPLE(EvalEnvironment, make))
    .registerMethod(POTHOS_FCN_TUPLE(EvalEnvironment, eval))
    .commit("Pothos/Util/EvalEnvironment");
