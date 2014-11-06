// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //getObjectMap
#include "EvalEngine.hpp"
#include "HostExplorer/HostExplorerDock.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"

EvalEngine::EvalEngine(QObject *parent):
    QObject(parent)
{
    connect(getObjectMap()["hostExplorerDock"], SIGNAL(hostUriListChanged(void)), this, SLOT(handleHostUriListChanged(void)));
    connect(getObjectMap()["affinityZonesDock"], SIGNAL(zoneChanged(void)), this, SLOT(handleAffinityZonesChanged(void)));
}

EvalEngine::~EvalEngine(void)
{
    return;
}

void EvalEngine::submitTopology(const GraphObjectList &graphObjects)
{
    
}

void EvalEngine::handleHostUriListChanged(void)
{
    auto dock = dynamic_cast<HostExplorerDock *>(getObjectMap()["hostExplorerDock"]);
}

void EvalEngine::handleAffinityZonesChanged(void)
{
    auto dock = dynamic_cast<AffinityZonesDock *>(getObjectMap()["affinityZonesDock"]);
}
