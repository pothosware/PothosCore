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
#include <QScrollArea>
#include <QScrollBar>
#include <QApplication> //control modifier
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <iostream>
#include <algorithm>

void GraphDraw::wheelEvent(QWheelEvent *event)
{
    const bool ctrlDown = QApplication::keyboardModifiers() & Qt::ControlModifier;
    if (not ctrlDown) return QWidget::wheelEvent(event);

    //ctrl was down, wheel event means zoom in or out:
    if (event->delta() > 0) getActionMap()["zoomIn"]->activate(QAction::Trigger);
    if (event->delta() < 0) getActionMap()["zoomOut"]->activate(QAction::Trigger);
}

void GraphDraw::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        const auto objs = this->getObjectsAtPos(event->localPos());
        if (not objs.isEmpty()) emit this->modifyProperties(objs.at(0));
    }
    QWidget::mouseDoubleClickEvent(event);
}

void GraphDraw::mousePressEvent(QMouseEvent *event)
{
    //record the conditions of this press event, nothing is changed
    if (event->button() == Qt::LeftButton)
    {
        _mouseLeftDown = true;
        _mouseLeftDownFirstPoint = event->localPos();
        _mouseLeftDownLastPoint = event->localPos();
        _selectionState = "pressed";
    }

    //Right-click will add a selection if nothing was selected.
    //This makes sure the right-click context menu sees a selection
    //when a graph object, that is not selected, is right-clicked.
    if (event->button() == Qt::RightButton)
    {
        const auto objs = this->getObjectsAtPos(event->localPos());
        size_t numSelected = 0;
        for (auto obj : objs)
        {
            if (obj->getSelected()) numSelected++;
        }
        if (numSelected == 0 and not objs.empty())
        {
            objs.back()->setSelected(true);
            this->render();
        }
    }

    QWidget::mousePressEvent(event);
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
    const bool ctrlDown = QApplication::keyboardModifiers() & Qt::ControlModifier;

    //handle the first move event transition from a press event
    if (_mouseLeftDown and _selectionState == "pressed")
    {
        auto objs = this->getObjectsAtPos(_mouseLeftDownFirstPoint);
        if (not objs.empty())
        {
            if (not objs.back()->getSelected()) this->doClickSelection(_mouseLeftDownFirstPoint);
            _selectionState = "move";
        }
        else
        {
            if (not ctrlDown) this->deselectAllObjs();
            _selectionState = "highlight";
        }
    }

    //deal with the dragging of graph objects
    if (_mouseLeftDown and _selectionState == "move")
    {
        for (auto obj : this->getObjectsSelected(~GRAPH_CONNECTION))
        {
            obj->move((event->localPos() - _mouseLeftDownLastPoint)/this->zoomScale());
            //keep the block within the drawing space
            QPointF newPoint = obj->getPosition()*this->zoomScale();
            if (newPoint.x() < 0) newPoint.setX(0);
            if (newPoint.y() < 0) newPoint.setY(0);
            if (newPoint.x() > this->size().width()) newPoint.setX(this->size().width()-1);
            if (newPoint.y() > this->size().height()) newPoint.setY(this->size().height()-1);
            obj->setPosition(newPoint/this->zoomScale());
        }
    }

    //store current position for tracking
    if (_mouseLeftDown)
    {
        _mouseLeftDownLastPoint = event->localPos();
        this->render();
    }

    //auto scroll near boundaries
    auto scrollArea = dynamic_cast<QScrollArea *>(this->parent()->parent());
    if (scrollArea != nullptr)
    {
        handleAutoScroll(scrollArea->horizontalScrollBar(), scrollArea->size().width(), event->localPos().x());
        handleAutoScroll(scrollArea->verticalScrollBar(), scrollArea->size().height(), event->localPos().y());
    }

    QWidget::mouseMoveEvent(event);
}

void GraphDraw::mouseReleaseEvent(QMouseEvent *event)
{
    //mouse released from a pressed state - alter selections at point
    if (event->button() == Qt::LeftButton and _selectionState == "pressed")
    {
        this->doClickSelection(_mouseLeftDownFirstPoint);
    }

    //mouse released from highlight box - alter selections with those in box
    if (event->button() == Qt::LeftButton and _selectionState == "highlight")
    {
        for (auto obj : this->getGraphObjects())
        {
            if (obj->isPointing(QRectF(_mouseLeftDownFirstPoint/this->zoomScale(), _mouseLeftDownLastPoint/this->zoomScale())))
            {
                obj->setSelected(true);
            }
        }
    }

    //emit the move event up to the graph editor
    if (event->button() == Qt::LeftButton and _selectionState == "move")
    {
        auto selected = getObjectsSelected(~GRAPH_CONNECTION);
        if (not selected.isEmpty()) emit stateChanged(GraphState("transform-move", tr("Move %1").arg(this->getSelectionDescription(~GRAPH_CONNECTION))));
    }

    if (event->button() == Qt::LeftButton)
    {
        _selectionState = "";
        _mouseLeftDown = false;
        this->render();
    }

    QWidget::mouseReleaseEvent(event);
}

void GraphDraw::deselectAllObjs(void)
{
    for (auto obj : this->getGraphObjects())
    {
        obj->setSelected(false);
    }
}

int GraphDraw::getMaxZIndex(void)
{
    int index = 0;
    for (auto obj : this->getGraphObjects())
    {
        index = std::max(index, obj->getZIndex());
    }
    return index;
}

GraphObjectList GraphDraw::getObjectsAtPos(const QPointF &pos, const int selectionFlags)
{
    GraphObjectList objectsAtPos;
    for (auto obj : this->getGraphObjects(selectionFlags))
    {
        if (obj->isPointing(pos/this->zoomScale())) objectsAtPos.push_back(obj);
    }
    return objectsAtPos;
}

GraphObjectList GraphDraw::getObjectsSelected(const int selectionFlags)
{
    GraphObjectList objectsSelected;
    for (auto obj : this->getGraphObjects(selectionFlags))
    {
        if (obj->getSelected()) objectsSelected.push_back(obj);
    }
    return objectsSelected;
}

static bool cmpGraphObjects(const GraphObject *lhs, const GraphObject *rhs)
{
    return lhs->getZIndex() < rhs->getZIndex();
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
    const auto objs = this->getObjectsAtPos(point);
    if (not objs.empty())
    {
        auto topObj = objs.back();
        bool newSel = true;
        //the selected object will have its selection inverted if its the only selection or ctrl
        if (this->getObjectsSelected().size() == 1 or ctrlDown) newSel = not topObj->getSelected();
        if (not ctrlDown) this->deselectAllObjs();
        topObj->setSelected(newSel);
        const int maxZIndex = this->getMaxZIndex();
        topObj->setZIndex(maxZIndex+1);
    }
    else if (not ctrlDown) this->deselectAllObjs();

    //nothing selected, clear the last selected endpoint
    if (objs.empty()) _lastClickSelectEp = GraphConnectionEndpoint();

    //connection creation logic
    if (not ctrlDown and not objs.empty())
    {
        auto topObj = objs.back();
        GraphConnectionEndpoint thisEp(topObj, topObj->isPointingToConnectable(point));

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

    emit this->selectionChanged(this->getObjectsSelected());
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
