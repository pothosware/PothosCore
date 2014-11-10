// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Proxy.hpp>
#include <Pothos/Exception.hpp>
#include <Poco/JSON/Object.h>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
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
    bool isGraphWidget;
    QString id;
    size_t uid;
    bool enabled;
    QString zone;
    std::map<QString, QString> properties;
    std::map<QString, Poco::JSON::Object::Ptr> paramDescs;
    Poco::JSON::Object::Ptr desc;
};

//! values to pass back to the gui thread to update the block
struct BlockStatus
{
    QPointer<GraphBlock> block;
    QPointer<QWidget> widget;
    std::map<QString, std::string> propertyTypeInfos;
    std::map<QString, QString> propertyErrorMsgs;
    QStringList blockErrorMsgs;
    Poco::JSON::Array::Ptr inPortDesc;
    Poco::JSON::Array::Ptr outPortDesc;
};

class BlockEval : public QObject
{
    Q_OBJECT
public:

    BlockEval(void);

    ~BlockEval(void);

    /*!
     * Does this information probably refer to this block?
     * We say probably, because this is a guess without UID.
     * Its an info match when the object ID matches,
     * and the block description path is identical.
     */
    bool isInfoMatch(const BlockInfo &info) const;

    /*!
     * Is the block eval in a usable state?
     * Environment active, no errors, ports queried...
     */
    bool isReady(void) const;

    /*!
     * Get the remote proxy block from the evaluator
     */
    Pothos::Proxy getProxyBlock(void) const;

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

    /*!
     * Perform update work after changes applied.
     */
    void update(void);

private slots:

    /*!
     * Update the block from the gui thread context.
     */
    void postStatusToBlock(const BlockStatus &status);

    /*!
     * Call block eval from the gui thread context.
     * This is required for blocks that have widgets.
     */
    void blockEvalInGUIContext(void);

private:

    //! critical change? need to make a new block
    bool hasCriticalChange(void) const;

    //! any setters that changed so we can re-call them
    std::vector<Poco::JSON::Object::Ptr> settersChangedList(void) const;

    //! detect a change in properties before vs after
    bool didPropKeyHaveChange(const QString &key) const;

    /*!
     * Create the remote block evaluator if needed.
     * Call evalProperty on all properties.
     * Record error conditions of each property.
     * Record the data type of each property.
     * \return true for success, false for error
     */
    bool updateAllProperties(void);

    void reportError(const std::string &action, const Pothos::Exception &ex);

    std::shared_ptr<EnvironmentEval> _newEnvironmentEval;
    std::shared_ptr<EnvironmentEval> _lastEnvironmentEval;
    std::shared_ptr<ThreadPoolEval> _newThreadPoolEval;
    std::shared_ptr<ThreadPoolEval> _lastThreadPoolEval;
    BlockInfo _newBlockInfo;
    BlockInfo _lastBlockInfo;

    //! tracking status in the eval thread context
    BlockStatus _lastBlockStatus;

    //remote block evaluator
    Pothos::Proxy _blockEval;
};
