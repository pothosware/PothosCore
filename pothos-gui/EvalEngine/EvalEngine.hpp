// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Environment.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QString>
#include <QObject>
#include <memory>
#include <condition_variable>
#include <mutex>
#include <map>
#include <thread>

class ZoneEngine;
class BlockEngine;
class GraphBlock;

/*!
 * Information about a connection between src and dst ports.
 * This is everything important we need to know about connections,
 * but extracted so we can access it in a thread-safe manner.
 */
struct ConnectionInfo
{
    std::string srcId, srcPort;
    std::string dstId, dstPort;
    std::string toString(void) const
    {
        return "("+srcId+"["+srcPort+"], "+dstId+"["+dstPort+"])";
    }
};

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

/*!
 * Topology information passed into the evaluation thread.
 */
struct TopologyInfo
{
    std::vector<BlockInfo> blockInfo;
    std::vector<ConnectionInfo> connectionInfo;
};

/*!
 * The EvalEngine is the entry point for submitting design changes.
 * Changes are evaluated asynchronously in background threads.
 * Post-evaluation events are handled in the main GUI thread.
 */
class EvalEngine : public QObject
{
    Q_OBJECT
public:

    EvalEngine(QObject *parent);

    ~EvalEngine(void);

    /*!
     * Submit the most recent version of the topology.
     * This lets us know which blocks in the cache are part of the design,
     * and how the blocks are connected by traversing breakers and connections.
     */
    void submitTopology(const GraphObjectList &graphObjects);

    /*!
     * Activate or deactivate the design.
     * If the activation fails, deactivateDesign() will be emitted.
     */
    void submitActivityState(const bool active);

signals:

    //! A failure occured, this is a notification to deactivate
    void deactivateDesign(void);

private slots:
    void handleHostUriListChanged(void);
    void handleAffinityZonesChanged(void);

private:

    static std::vector<ConnectionInfo> getConnectionInfo(const GraphObjectList &graphObjects);

    //async event handler thread hooks
    void startEvalThread(void);
    void stopEvalThread(void);
    void runEvalThread(void);
    void handleEvent(const TopologyInfo &info);

    //thread communication event queue
    std::mutex _eventMutex;
    std::condition_variable _eventCond;
    TopologyInfo _evalThreadInfo;
    bool _evalThreadRunning;
    std::thread _evalThread;

    //active evaluation hooks
    void reEvalAll(void);
    std::map<std::string, std::shared_ptr<ZoneEngine>> _zoneEngines;
    std::map<QString, std::shared_ptr<BlockEngine>> _blockEngines;
};
