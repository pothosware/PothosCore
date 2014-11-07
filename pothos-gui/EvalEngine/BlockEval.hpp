// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include <Poco/JSON/Object.h>
#include <QObject>
#include <QPointer>
#include <QString>
#include <memory>

class EnvironmentEval;
class ThreadPoolEval;
class GraphBlock;

/*!
 * Information about a block that is used for background evaluation.
 * This is everything important we need to know about a block,
 * but extracted so we can access it in a thread-safe manner.
 * We store the GraphBlock in a QPointer which can only be
 * looked at in the GUI thread context. Which we will use
 * to update the block after evaluation from the GUI thread.
 */
struct BlockInfo
{
    QPointer<GraphBlock> block;
    QString id;
    QString zone;
    std::map<QString, QString> properties;
    std::map<QString, Poco::JSON::Object::Ptr> paramDescs;
    Poco::JSON::Object::Ptr desc;
};

class BlockEval : public QObject
{
    Q_OBJECT
public:

    BlockEval(void);

    ~BlockEval(void);

    /*!
     * Called under re-eval to apply the latest info.
     * This call should take the info and not process.
     */
    void acceptInfo(const BlockInfo &info);

    /*!
     * Called under re-eval to apply the latest environment.
     * This call should take the info and not process.
     */
    void acceptEnvironment(const std::shared_ptr<EnvironmentEval> &env);

    /*!
     * Called under re-eval to apply the latest thread pool.
     * This call should take the info and not process.
     */
    void acceptThreadPool(const std::shared_ptr<ThreadPoolEval> &tp);

    void update(void);

private:

    std::shared_ptr<EnvironmentEval> _environmentEval;
    std::shared_ptr<ThreadPoolEval> _threadPoolEval;
    BlockInfo _lastBlockInfo;
};
