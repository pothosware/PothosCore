// Copyright (c) 2013-2018 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <string>
#include <Pothos/Init.hpp>
#include <Poco/Util/ServerApplication.h>
#include <utility> //pair
#include <vector>

class PothosUtilBase : public Poco::Util::ServerApplication
{
public:
    virtual ~PothosUtilBase(void){}
    void printSystemInfo(const std::string &, const std::string &);
    void printModuleInfo(const std::string &, const std::string &);
    void printDeviceInfo(void);
    void printPluginTree(const std::string &, const std::string &);
    void selfTests(const std::string &, const std::string &);
    void selfTestOne(const std::string &, const std::string &);
    void proxyServer(const std::string &, const std::string &);
    void loadModule(const std::string &, const std::string &);
    void runTopology(void);
    void docParse(const std::vector<std::string> &);
    void listModules(const std::string &, const std::string &);
    void printProxyEnvironmentInfo(const std::string &, const std::string &);

    //! Variables passed in via the --vars option
    std::vector<std::pair<std::string, std::string>> _vars;
};
