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

    /*!
     * Called under re-eval to apply the latest config.
     * This call should take the info and not process.
     */
    void acceptConfig(const QString &zoneName, const Poco::JSON::Object::Ptr &config);

    /*!
     * Deal with changes from the latest config.
     */
    void update(void);

    //! Shared method to parse the zone config into host uri and process name
    static HostProcPair getHostProcFromConfig(const QString &zoneName, const Poco::JSON::Object::Ptr &config);

    //! Get access to the proxy environment
    Pothos::ProxyEnvironment::Sptr getEnv(void) const
    {
        return _env;
    }

    //! Get access to the expression evaluator
    Pothos::Proxy getEval(void) const
    {
        return _eval;
    }

    //! An error caused the environment to go into failure state
    bool isFailureState(void) const
    {
        return _failureState;
    }

    //! Get the error message associated with the failure state
    const QString &getErrorMsg(void) const
    {
        return _errorMsg;
    }

private:
    Pothos::ProxyEnvironment::Sptr makeEnvironment(void);

    QString _zoneName;
    Poco::JSON::Object::Ptr _config;
    Pothos::ProxyEnvironment::Sptr _env;
    Pothos::Proxy _eval;
    bool _failureState;
    QString _errorMsg;
};
