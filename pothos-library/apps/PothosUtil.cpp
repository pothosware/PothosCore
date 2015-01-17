// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosUtil.hpp"
#include <Poco/Util/Application.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/IntValidator.h>
#include <Pothos/System.hpp>
#include <iostream>

class PothosUtil : public PothosUtilBase
{
public:
    PothosUtil(int argc):
        _helpRequested(argc <= 1),
        _docParseRequested(false),
        _runTopologyRequested(false)
    {
        this->setUnixOptions(true); //always unix style --option

        //restore logging config after Poco::Util::Application::initialize() obliterates it
        Pothos::System::Logger::setupDefaultLogging();
    }

    ~PothosUtil()
    {
        return;
    }

protected:

    void defineOptions(Poco::Util::OptionSet &options)
    {
        Poco::Util::Application::defineOptions(options);

        options.addOption(Poco::Util::Option("help", "h", "display argument help information")
            .required(false)
            .repeatable(false));

        options.addOption(Poco::Util::Option("system-info", "", "display system information")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<PothosUtil>(this, &PothosUtil::printSystemInfo)));

        options.addOption(Poco::Util::Option("device-info", "", "display device information")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<PothosUtil>(this, &PothosUtil::printDeviceInfo)));

        options.addOption(Poco::Util::Option("print-tree", "", "display plugin tree")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<PothosUtil>(this, &PothosUtil::printTree)));

        options.addOption(Poco::Util::Option("proxy-server", "", "run the proxy server, tcp://bindHost:bindPort")
            .required(false)
            .repeatable(false)
            .argument("URI")
            .callback(Poco::Util::OptionCallback<PothosUtil>(this, &PothosUtil::proxyServer)));

        options.addOption(Poco::Util::Option("load-module", "", "test load a library module")
            .required(false)
            .repeatable(false)
            .argument("modulePath")
            .callback(Poco::Util::OptionCallback<PothosUtil>(this, &PothosUtil::loadModule)));

        options.addOption(Poco::Util::Option("run-topology", "", "run a topology from a JSON description")
            .required(false)
            .repeatable(false)
            .argument("inputFile")
            .binding("inputFile"));

        options.addOption(Poco::Util::Option("run-duration", "", "run the topology for the duration in seconds")
            .required(false)
            .repeatable(false)
            .argument("runDuration")
            .binding("runDuration"));

        options.addOption(Poco::Util::Option("self-tests", "", "run all plugin self tests")
            .required(false)
            .repeatable(false)
            .callback(Poco::Util::OptionCallback<PothosUtil>(this, &PothosUtil::selfTests)));

        options.addOption(Poco::Util::Option("self-tests-at", "", "run all plugin self tests given a subtree")
            .required(false)
            .repeatable(false)
            .argument("pluginPath")
            .callback(Poco::Util::OptionCallback<PothosUtil>(this, &PothosUtil::selfTests)));

        options.addOption(Poco::Util::Option("self-test1", "", "run a particular plugin self test")
            .required(false)
            .repeatable(false)
            .argument("pluginPath")
            .callback(Poco::Util::OptionCallback<PothosUtil>(this, &PothosUtil::selfTestOne)));

        options.addOption(Poco::Util::Option("success-code", "", "the success status return code (default 0)")
            .required(false)
            .repeatable(false)
            .argument("successCode")
            .validator(new Poco::Util::IntValidator(0, 255))
            .binding("successCode"));

        options.addOption(Poco::Util::Option("require-active", "", "proxy server shuts off without active clients")
            .required(false)
            .repeatable(false)
            .binding("requireActive"));

        options.addOption(Poco::Util::Option("output", "", "specify an output file (used by various options)")
            .required(false)
            .repeatable(false)
            .argument("outputFile")
            .binding("outputFile"));

        options.addOption(Poco::Util::Option("doc-parse", "", "parse specified files for documentation markup")
            .required(false)
            .repeatable(false));
    }

    void handleOption(const std::string &name, const std::string &value)
    {
        ServerApplication::handleOption(name, value);
        if (name == "help") _helpRequested = true;
        if (name == "doc-parse") _docParseRequested = true;
        if (name == "run-topology") _runTopologyRequested = true;
        if (name == "help") this->stopOptionsProcessing();
    }

    void printSystemInfo(const std::string &, const std::string &)
    {
        std::cout << "API Version: " << Pothos::System::getApiVersion() << std::endl;
        std::cout << "ABI Version: " << Pothos::System::getAbiVersion() << std::endl;
        std::cout << "Root Path: " << Pothos::System::getRootPath() << std::endl;
        std::cout << "Data Path: " << Pothos::System::getDataPath() << std::endl;
        std::cout << "User Data: " << Pothos::System::getUserDataPath() << std::endl;
        std::cout << "User Config: " << Pothos::System::getUserConfigPath() << std::endl;
        std::cout << "Runtime Library: " << Pothos::System::getPothosRuntimeLibraryPath() << std::endl;
        std::cout << "Util Executable: " << Pothos::System::getPothosUtilExecutablePath() << std::endl;
        std::cout << "Dev Include Path: " << Pothos::System::getPothosDevIncludePath() << std::endl;
        std::cout << "Dev Library Path: " << Pothos::System::getPothosDevLibraryPath() << std::endl;
    }

    void displayHelp(void)
    {
        Poco::Util::HelpFormatter helpFormatter(this->options());
        helpFormatter.setUnixStyle(true); //always unix style --option
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("\n"
            "PothosUtil is a helper exectuable for the Pothos Library; "
            "used both internally by the library, and externally by the user. "
            "The util provides a command-line interface for the self-tests, "
            "and can spawn servers and daemons for remote interfaces. ");
        helpFormatter.format(std::cout);
    }

    int main(const std::vector<std::string> &args)
    {
        try
        {
            if (_helpRequested) this->displayHelp();
            else if (_docParseRequested) this->docParse(args);
            else if (_runTopologyRequested) this->runTopology();
        }
        catch(const Pothos::Exception &ex)
        {
            std::cerr << ex.displayText() << std::endl;
            std::cout << std::endl;
            throw ex;
        }
        return Poco::Util::Application::EXIT_OK;
    }

private:
    bool _helpRequested;
    bool _docParseRequested;
    bool _runTopologyRequested;
};

int main(int argc, char *argv[])
{
    POTHOS_EXCEPTION_TRY
    {
        PothosUtil app(argc);
        const int ret = app.run(argc, argv);
        if (ret == 0) return app.config().getInt("successCode", 0);
        return ret;
    }
    POTHOS_EXCEPTION_CATCH(const Pothos::Exception &ex)
    {
        std::cerr << ex.displayText() << std::endl;
        std::cout << std::endl;
    }
    return Poco::Util::Application::EXIT_USAGE;
}
