// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphBlockImpl.hpp"
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Util/Compiler.hpp>
#include <Pothos/Util/EvalInterface.hpp>
#include <Poco/Format.h>
#include <Poco/DigestStream.h>
#include <Poco/MD5Engine.h>
#include <Poco/TemporaryFile.h>
#include <Poco/ClassLoader.h>
#include <Poco/NumberParser.h>
#include <fstream>
#include <iostream>

//cache expression to obj
//cache objarray+path to block

void GraphBlock::initPropertiesFromDesc(void)
{
    const auto &blockDesc = _impl->blockDesc;

    //extract the name or title from the description
    const auto name = blockDesc->get("name").convert<std::string>();
    this->setTitle(QString::fromStdString(name));

    //extract the params or properties from the description
    const auto params = blockDesc->getArray("params");
    if (params) for (size_t i = 0; i < params->size(); i++)
    {
        const auto param = params->getObject(i);
        const auto key = QString::fromStdString(param->get("key").convert<std::string>());
        const auto name = QString::fromStdString(param->get("name").convert<std::string>());
        this->addProperty(GraphBlockProp(key, name));

        const auto options = param->getArray("options");
        if (param->has("default"))
        {
            this->setPropertyValue(key, QString::fromStdString(
                param->get("default").convert<std::string>()));
        }
        else if (options and options->size() > 0)
        {
            this->setPropertyValue(key, QString::fromStdString(
                options->getObject(0)->get("value").convert<std::string>()));
        }

        if (param->has("preview"))
        {
            const auto prev = param->get("preview").convert<std::string>();
            this->setPropertyPreview(key, prev == "enabled");
        }
    }
}

static void initGraphBlockPortsFromBlock(GraphBlock *self, Pothos::Block *b)
{
    for (const auto &portKey : b->inputPortNames())
    {
        auto port = b->input(portKey);
        if (port->isSlot()) continue;
        std::string portName = portKey;
        if (port->index() >= 0) portName = "in" + portName;
        self->addInputPort(GraphBlockPort(
            QString::fromStdString(portKey),
            QString::fromStdString(portName)));
    }

    for (const auto &portKey : b->outputPortNames())
    {
        auto port = b->output(portKey);
        if (port->isSignal()) continue;
        std::string portName = portKey;
        if (port->index() >= 0) portName = "out" + portName;
        self->addOutputPort(GraphBlockPort(
            QString::fromStdString(portKey),
            QString::fromStdString(portName)));
    }
}

/*
void GraphBlock::update(void)
{
    assert(_impl->blockDesc);
    const auto &blockDesc = _impl->blockDesc;

    std::ostringstream code;

    //create a function to return factory args
    code << std::endl;
    code << "#include <Pothos/Framework.hpp>" << std::endl;
    code << "#include <vector>" << std::endl;
    code << std::endl;
    code << "static std::vector<Pothos::Object> evalArgs(void)" << std::endl;
    code << "{" << std::endl;
    code << "using namespace Pothos;" << std::endl;
    for (const auto &prop : this->getProperties())
    {
        code << Poco::format("const auto %s = %s;",
            prop.getKey().toStdString(),
            this->getPropertyValue(prop.getKey()).toStdString()) << std::endl;
    }
    code << "std::vector<Pothos::Object> __args;" << std::endl;
    const auto args = blockDesc->getArray("args");
    if (args) for (size_t i = 0; i < args->size(); i++)
    {
        code << Poco::format("__args.emplace_back(%s);", args->get(i).convert<std::string>()) << std::endl;
    }
    code << "return __args;" << std::endl;
    code << "}" << std::endl;

    //create a unique symbol name
    Poco::MD5Engine md5; md5.update(code.str());
    const auto symName = "Eval_"+Poco::DigestEngine::digestToHex(md5.digest());

    //create the source to eval the string
    code << std::endl;
    code << "#include <Pothos/Util/EvalInterface.hpp>" << std::endl;
    code << "#include <Poco/ClassLibrary.h>" << std::endl;
    code << "struct " << symName << " : Pothos::Util::EvalInterface" << std::endl;
    code << "{" << std::endl;
    code << "    Pothos::Object eval(void) const" << std::endl;
    code << "    {return Pothos::Object(evalArgs());}" << std::endl;
    code << "};" << std::endl;
    code << "POCO_BEGIN_MANIFEST(Pothos::Util::EvalInterface)" << std::endl;
    code << "    POCO_EXPORT_CLASS(" << symName << ")" << std::endl;
    code << "POCO_END_MANIFEST" << std::endl;
    code << std::endl;

    //std::cout << code.str() << std::endl;

    //perform compilation
    Pothos::Util::CompilerArgs compilerArgs = Pothos::Util::CompilerArgs::defaultDevEnv();
    compilerArgs.sources.push_back(code.str());
    const auto compiler = Pothos::Util::Compiler::make();
    std::string outMod;
    try
    {
        outMod = compiler->compileCppModule(compilerArgs);
    }
    catch(const Pothos::Exception &ex)
    {
        std::cerr << ex.displayText() << std::endl;
        throw ex;
    }

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
        std::cerr << ex.displayText() << std::endl;
        throw ex;
    }

    //extract the symbol and call its evaluation routine
    Pothos::Util::EvalInterface* ev = loader.create(symName);
    Pothos::Object result = ev->eval();
    //std::cout << "result type " << result.type().name() << std::endl;
    delete ev;

    try
    {
        const auto factory = Pothos::BlockRegistry::lookup(blockDesc->get("path").convert<std::string>());
        const auto &opaqueArgs = result.extract<std::vector<Pothos::Object>>();
        auto b = factory.opaqueCall(opaqueArgs.data(), opaqueArgs.size()).extract<Pothos::Block *>();
        initGraphBlockPortsFromBlock(this, b);
        delete b;

    }
    catch (const Pothos::Exception &ex)
    {
        //TODO logger
        std::cerr << ex.displayText() << std::endl;
        throw ex;
    }

    result = Pothos::Object(); //stop holding onto module memory
    loader.unloadLibrary(outPath);
    Poco::File(outPath).remove();
}
*/

/***********************************************************************
 * non JIT update impl for testing
 **********************************************************************/
static Pothos::Object parseOne(const std::string &val)
{
    if (val.size() >= 2 and val.front() == '"' and val.back() == '"') return Pothos::Object(val.substr(1, val.size()-2));
    if (val == "true") return Pothos::Object(true);
    if (val == "false") return Pothos::Object(false);
    try {return Pothos::Object(Poco::NumberParser::parseUnsigned64(val));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parse64(val));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parseHex64(val));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parseFloat(val));}
    catch (const Poco::SyntaxException &){}
    try {return Pothos::Object(Poco::NumberParser::parse(val));}
    catch (const Poco::SyntaxException &){}
    std::cerr << ("cant parse " + val) << std::endl;
    throw Pothos::Exception("GraphBlock::update()", "cant parse " + val);
}

void GraphBlock::update(void)
{
    assert(_impl->blockDesc);
    const auto &blockDesc = _impl->blockDesc;

    std::vector<Pothos::Object> opaqueArgs;

    for (const auto &prop : this->getProperties())
    {
        const auto val = this->getPropertyValue(prop.getKey()).toStdString();
        opaqueArgs.push_back(parseOne(val));
    }

    try
    {
        const auto factory = Pothos::BlockRegistry::lookup(blockDesc->get("path").convert<std::string>());
        auto b = factory.opaqueCall(opaqueArgs.data(), opaqueArgs.size()).extract<Pothos::Block *>();
        initGraphBlockPortsFromBlock(this, b);
        delete b;

    }
    catch (const Pothos::Exception &ex)
    {
        //TODO logger
        std::cerr << ex.displayText() << std::endl;
        throw ex;
    }
}
