// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Proxy.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Poco/JSON/Object.h>
#include <QObject>
#include <QString>
#include <memory>
#include <set>

class EnvironmentEval;

class ThreadPoolEval : public QObject
{
    Q_OBJECT
public:

    ThreadPoolEval(void);

    ~ThreadPoolEval(void);

    /*!
     * Called under re-eval to apply the latest config.
     * This call should take the info and not process.
     */
    void acceptConfig(const Poco::JSON::Object::Ptr &config);

    /*!
     * Called under re-eval to apply the latest environment.
     * This call should take the info and not process.
     */
    void acceptEnvironment(const std::shared_ptr<EnvironmentEval> &env);

    /*!
     * Deal with changes from the latest config.
     */
    void update(void);

    //! Get the thread pool that will be set on blocks
    Pothos::Proxy getThreadPool(void) const
    {
        return _threadPool;
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

    Pothos::Proxy makeThreadPool(void);

    //Tracking state for the eval environment:
    //Also stash the actual proxy environment here.
    //The proxy environment provided by eval may change,
    //which we will use to determine pool re-evaluation.
    std::shared_ptr<EnvironmentEval> _newEnvironmentEval;
    std::shared_ptr<EnvironmentEval> _lastEnvironmentEval;
    Pothos::ProxyEnvironment::Sptr _newEnvironment;
    Pothos::ProxyEnvironment::Sptr _lastEnvironment;

    //Tracking state for the thread pool configuration.
    //A change in config merritts making a new thread pool.
    Poco::JSON::Object::Ptr _newZoneConfig;
    Poco::JSON::Object::Ptr _lastZoneConfig;

    Pothos::Proxy _threadPool;
    bool _failureState;
    QString _errorMsg;
};
