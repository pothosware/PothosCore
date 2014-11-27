// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QObject>
#include <chrono>

class QThread;
class QTimer;
class EvalEngineImpl;
class AffinityZonesDock;
class QSignalMapper;

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

public slots:

    /*!
     * Submit the most recent version of the topology.
     * This lets us know which blocks in the cache are part of the design,
     * and how the blocks are connected by traversing breakers and connections.
     */
    void submitTopology(const GraphObjectList &graphObjects);

    /*!
     * Submit a set of graph objects for re-evaluation.
     * The state of these objects will be cleared and re-processed.
     */
    void submitReeval(const GraphObjectList &graphObjects);

    /*!
     * Activate or deactivate the design.
     * If the activation fails, deactivateDesign() will be emitted.
     */
    void submitActivateTopology(const bool active);

    /*!
     * Submit individual graph block for re-eval.
     */
    void submitBlock(QObject *block);

    //! query the dot markup for the active topology
    std::string getTopologyDotMarkup(const std::string &config);

signals:

    //! A failure occured, this is a notification to deactivate
    void deactivateDesign(void);

private slots:
    void handleAffinityZonesChanged(void);
    void handleEvalThreadHeartBeat(void);
    void handleMonitorTimeout(void);

private:
    QThread *_thread;
    QTimer *_monitorTimer;
    EvalEngineImpl *_impl;
    QSignalMapper *_blockEvalMapper;
    AffinityZonesDock *_affinityDock;
    std::chrono::system_clock::time_point _lastHeartBeat;
};
