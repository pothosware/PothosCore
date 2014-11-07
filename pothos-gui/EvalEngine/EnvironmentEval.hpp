// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Poco/JSON/Object.h>
#include <QObject>
#include <QString>
#include <memory>
#include <string>
#include <utility>

typedef std::pair<std::string, std::string> HostProcPair;

class EnvironmentEval : public QObject
{
    Q_OBJECT
public:

    EnvironmentEval(void);

    ~EnvironmentEval(void);

    void evalExpr(const QString &expr);

    bool isEnvironmentAlive(void);

    /*!
     * Called under re-eval to apply the latest config.
     * This call should take the info and not process.
     */
    void acceptConfig(const Poco::JSON::Object::Ptr &config);

    /*!
     * Deal with changes from the latest config.
     */
    void update(void);

    static HostProcPair getHostProcFromConfig(const Poco::JSON::Object::Ptr &config)
    {
        auto hostUri = config?config->getValue<std::string>("hostUri"):"tcp://localhost";
        auto processName = config?config->getValue<std::string>("processName"):"";
        return std::make_pair(hostUri, processName);
    }

private:
    Pothos::ProxyEnvironment::Sptr _env;
    Pothos::Proxy _eval;
};
