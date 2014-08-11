// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //action maps
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include "GraphEditor/Constants.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include <Pothos/Exception.hpp>
#include <Poco/Logger.h>
#include <QApplication> //control modifier
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QScrollBar>
#include <iostream>
#include <algorithm>

static const int SELECTION_STATE_NONE = 0;
static const int SELECTION_STATE_PRESS = 1;
static const int SELECTION_STATE_MOVE = 2;

void GraphDraw::wheelEvent(QWheelEvent *event)
{
    const bool ctrlDown = QApplication::keyboardModifiers() & Qt::ControlModifier;
    if (not ctrlDown) return QGraphicsView::wheelEvent(event);

    //ctrl was down, wheel event means zoom in or out:
    if (event->delta() > 0) getActionMap()["zoomIn"]->activate(QAction::Trigger);
    if (event->delta() < 0) getActionMap()["zoomOut"]->activate(QAction::Trigger);
}

void GraphDraw::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        const auto objs = this->items(event->pos());
        if (not objs.isEmpty()) emit this->modifyProperties(dynamic_cast<GraphObject *>(objs.at(0)));
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphDraw::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);

    //record the conditions of this press event, nothing is changed
    if (event->button() == Qt::LeftButton)
    {
        _selectionState = SELECTION_STATE_PRESS;

        //make the clicked object topmost
        const auto objs = this->items(event->pos());
        if (not objs.empty()) objs.front()->setZValue(this->getMaxZValue()+1);
    }

    //Right-click will add a selection if nothing was selected.
    //This makes sure the right-click context menu sees a selection
    //when a graph object, that is not selected, is right-clicked.
    if (event->button() == Qt::RightButton)
    {
        const auto objs = this->items(event->pos());
        size_t numSelected = 0;
        for (auto obj : objs)
        {
            if (obj->isSelected()) numSelected++;
        }
        if (numSelected == 0 and not objs.empty())
        {
            objs.front()->setSelected(true);
        }
    }

    this->render();
}

static void handleAutoScroll(QScrollBar *bar, const qreal length, const qreal offset)
{
    const qreal delta = offset - bar->value();
    if (delta + GraphDrawScrollFudge > length)
    {
        bar->setValue(std::min(bar->maximum(), int(bar->value() + (delta + GraphDrawScrollFudge - length)/2)));
    }
    if (delta - GraphDrawScrollFudge < 0)
    {
        bar->setValue(std::max(0, int(bar->value() + (delta - GraphDrawScrollFudge)/2)));
    }
}

void GraphDraw::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);

    //handle the first move event transition from a press event
    if (_selectionState == SELECTION_STATE_PRESS)
    {
        _selectionState = (this->items(event->pos()).empty())? SELECTION_STATE_NONE : SELECTION_STATE_MOVE;
    }

    this->render();

    //auto scroll near boundaries
    handleAutoScroll(this->horizontalScrollBar(), this->size().width(), this->mapToScene(event->pos()).x());
    handleAutoScroll(this->verticalScrollBar(), this->size().height(), this->mapToScene(event->pos()).y());
}

void GraphDraw::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

    //mouse released from a pressed state - alter selections at point
    if (_selectionState == SELECTION_STATE_PRESS)
    {
        _selectionState = SELECTION_STATE_NONE;
        this->doClickSelection(this->mapToScene(event->pos()));
    }

    //emit the move event up to the graph editor
    if (_selectionState == SELECTION_STATE_MOVE)
    {
        _selectionState = SELECTION_STATE_NONE;
        auto selected = getObjectsSelected(~GRAPH_CONNECTION);
        if (not selected.isEmpty()) emit stateChanged(GraphState("transform-move", tr("Move %1").arg(this->getSelectionDescription(~GRAPH_CONNECTION))));
    }

    this->render();
}

void GraphDraw::deselectAllObjs(void)
{
    for (auto obj : this->getGraphObjects())
    {
        obj->setSelected(false);
    }
}

qreal GraphDraw::getMaxZValue(void)
{
    qreal index = 0;
    bool first = true;
    for (auto obj : this->getGraphObjects())
    {
        if (first) index = obj->zValue();
        index = std::max(index, obj->zValue());
        first = false;
    }
    return index;
}

GraphObjectList GraphDraw::getObjectsSelected(const int selectionFlags)
{
    GraphObjectList objectsSelected;
    for (auto obj : this->getGraphObjects(selectionFlags))
    {
        if (obj->isSelected()) objectsSelected.push_back(obj);
    }
    return objectsSelected;
}

static bool cmpGraphObjects(const GraphObject *lhs, const GraphObject *rhs)
{
    return lhs->zValue() < rhs->zValue();
}

GraphObjectList GraphDraw::getGraphObjects(const int selectionFlags)
{
    GraphObjectList l;
    for (auto child : this->children())
    {
        auto o = dynamic_cast<GraphObject *>(child);
        if (o == nullptr) continue;
        if (((selectionFlags & GRAPH_BLOCK) != 0) and (dynamic_cast<GraphBlock *>(o) != nullptr)) l.push_back(o);
        if (((selectionFlags & GRAPH_BREAKER) != 0) and (dynamic_cast<GraphBreaker *>(o) != nullptr)) l.push_back(o);
        if (((selectionFlags & GRAPH_CONNECTION) != 0) and (dynamic_cast<GraphConnection *>(o) != nullptr)) l.push_back(o);
    }
    std::sort(l.begin(), l.end(), &cmpGraphObjects);
    return l;
}

void GraphDraw::doClickSelection(const QPointF &point)
{
    const bool ctrlDown = QApplication::keyboardModifiers() & Qt::ControlModifier;
    const auto objs = this->items(this->mapFromScene(point));

    //nothing selected, clear the last selected endpoint
    if (objs.empty()) _lastClickSelectEp = GraphConnectionEndpoint();

    //connection creation logic
    if (not ctrlDown and not objs.empty())
    {
        auto topObj = dynamic_cast<GraphObject *>(objs.front());
        if (topObj == nullptr) return;
        GraphConnectionEndpoint thisEp(topObj, topObj->isPointingToConnectable(topObj->mapFromParent(point)));

        //valid keys, attempt to make a connection
        QPointer<GraphConnection> conn;
        if (thisEp.isValid() and _lastClickSelectEp.isValid() and not (thisEp == _lastClickSelectEp) and //end points valid
            (_lastClickSelectEp.getConnectableAttrs().direction == GRAPH_CONN_OUTPUT or _lastClickSelectEp.getConnectableAttrs().direction == GRAPH_CONN_SIGNAL) and //last endpoint is output
            (thisEp.getConnectableAttrs().direction == GRAPH_CONN_INPUT or thisEp.getConnectableAttrs().direction == GRAPH_CONN_SLOT)) //this click endpoint is input
        {
            try
            {
                conn = this->getGraphEditor()->makeConnection(thisEp, _lastClickSelectEp);
                emit stateChanged(GraphState("connect-arrow", tr("Connect %1[%2] to %3[%4]").arg(
                    conn->getOutputEndpoint().getObj()->getId(),
                    conn->getOutputEndpoint().getKey().id,
                    conn->getInputEndpoint().getObj()->getId(),
                    conn->getInputEndpoint().getKey().id
                )));
            }
            catch (const Pothos::Exception &ex)
            {
                poco_warning(Poco::Logger::get("PothosGui.GraphDraw.connect"), Poco::format("Cannot connect port %s[%s] to port %s[%s]: %s",
                    _lastClickSelectEp.getObj()->getId().toStdString(),
                    _lastClickSelectEp.getKey().id.toStdString(),
                    thisEp.getObj()->getId().toStdString(),
                    thisEp.getKey().id.toStdString(),
                    ex.message()));
            }
        }

        //cleanup after new connection
        if (not conn.isNull())
        {
            _lastClickSelectEp = GraphConnectionEndpoint();
            this->deselectAllObjs();
        }
        //otherwise save the click select
        else
        {
            _lastClickSelectEp = thisEp;
        }
    }
}

QString GraphDraw::getSelectionDescription(const int selectionFlags)
{
    //generate names based on the selected objects
    const auto selected = this->getObjectsSelected(selectionFlags);
    if (selected.isEmpty()) return tr("no selection");

    //if a single connection is selected, pretty print its endpoint IDs
    if (selected.size() == 1)
    {
        auto conn = dynamic_cast<GraphConnection *>(selected.at(0));
        if (conn == nullptr) return selected.at(0)->getId();
        return tr("%1[%2] to %3[%4]").arg(
            conn->getOutputEndpoint().getObj()->getId(),
            conn->getOutputEndpoint().getKey().id,
            conn->getInputEndpoint().getObj()->getId(),
            conn->getInputEndpoint().getKey().id
        );
    }
    return tr("selected");
}

GraphObject *GraphDraw::getObjectById(const QString &id, const int selectionFlags)
{
    for (auto obj : this->getGraphObjects(selectionFlags))
    {
        if (obj->getId() == id) return obj;
    }
    return nullptr;
}
