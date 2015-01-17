// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#ifndef __INCLUDED_POTHOS_UTIL_HPP__
#define __INCLUDED_POTHOS_UTIL_HPP__

#include <string>
#include <Pothos/Init.hpp>
#include <Poco/Util/ServerApplication.h>

class PothosUtilBase : public Poco::Util::ServerApplication
{
public:
    virtual ~PothosUtilBase(void){}
    void printDeviceInfo(void);
    void printTree(const std::string &, const std::string &);
    void selfTests(const std::string &, const std::string &);
    void selfTestOne(const std::string &, const std::string &);
    void proxyServer(const std::string &, const std::string &);
    void loadModule(const std::string &, const std::string &);
    void runTopology(void);
    void docParse(const std::vector<std::string> &);
};

#endif /*__INCLUDED_POTHOS_UTIL_HPP__*/
