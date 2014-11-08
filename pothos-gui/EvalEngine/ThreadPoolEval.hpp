// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Proxy.hpp>
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

private:

    Pothos::Proxy makeThreadPool(void);

    std::shared_ptr<EnvironmentEval> _newEnvironmentEval;
    std::shared_ptr<EnvironmentEval> _lastEnvironmentEval;

    Poco::JSON::Object::Ptr _newZoneConfig;
    Poco::JSON::Object::Ptr _lastZoneConfig;
    Pothos::Proxy _threadPool;
};
