// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QString>
#include <QObject>

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
    void handleAffinityZonesChanged(void);

private:

    void reEvalAll(void);

    //async event handler thread hooks
    void startEvalThread(void);
    void stopEvalThread(void);
    void runEvalThread(void);
    void submitInfo(void);

    //private data
    struct Impl;
    Impl *_impl;
};
