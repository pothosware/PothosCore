// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Util/EvalInterface.hpp>
#include <Pothos/Object.hpp>
#include <Poco/TemporaryFile.h>
#include <Poco/ClassLoader.h>
#include <Poco/DigestStream.h>
#include <Poco/MD5Engine.h>
#include <Poco/File.h>
#include <Poco/NumberParser.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

/*!
 * The evaluation environment can evaluate and inspect expressions.
 * Expressions are considered to be valid C++ expressions involving
 * bools, integers, strings, floats, stl classes...
 */
class EvalEnvironment
{
public:
    EvalEnvironment(void)
    {
        return;
    }

    /*!
     * Try to evaluate an expression in this environment.
     * \param expression a string expression
     * \throws Pothos::Exception with the evaluation error message
     * \return a proxy object representing the evaluated expression
     */
    Pothos::Object eval(const std::string &);

    /*!
     * Register a constant that will be used in other expressions.
     * For an expression like 2*x, then "x" would be a constant.
     */
    void registerConstant(const std::string &key, const std::string &expression)
    {
        //TODO
    }
};

Pothos::Object EvalEnvironment::eval(const std::string &expr)
{
    //try simple evaluations first
    if (expr.size() >= 2 and expr.front() == '"' and expr.back() == '"') return Pothos::Object(expr.substr(1, expr.size()-2));
    if (expr == "true") return Pothos::Object(true);
    if (expr == "false") return Pothos::Object(false);
    try {return Pothos::Object(Poco::NumberParser::parseUnsigned64(expr));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parse64(expr));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parseHex64(expr));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parseFloat(expr));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parse(expr));}
    catch (const Poco::SyntaxException &){}

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
    Poco::ClassLoader<Pothos::Util::EvalInterface> loader;
    try
    {
        loader.loadLibrary(outPath);
    }
    catch (const Poco::Exception &ex)
    {
        Poco::File(outPath).remove();
        throw Pothos::Exception("EvalEnvironment::eval", ex.displayText());
    }

    //extract the symbol and call its evaluation routine
    std::stringstream ss;
    {
        Pothos::Util::EvalInterface* eval0 = loader.create("Eval_"+symName);
        Pothos::Object result = eval0->eval();
        result.serialize(ss);
        delete eval0;
    }

    //cleanup
    loader.unloadLibrary(outPath);
    Poco::File(outPath).remove();

    Pothos::Object result;
    result.deserialize(ss);
    return result;
}

#include <Pothos/Managed.hpp>

static auto managedEvalEnvironment = Pothos::ManagedClass()
    .registerConstructor<EvalEnvironment>()
    .registerMethod(POTHOS_FCN_TUPLE(EvalEnvironment, eval))
    .commit("Pothos/Gui/EvalEnvironment");
