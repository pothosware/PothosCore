// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //action and object map
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphEditor/Constants.hpp"
#include <Poco/JSON/Parser.h>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QAction>
#include <QChildEvent>
#include <QScrollBar>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <iostream>
#include <cassert>
#include <limits>
#include <algorithm> //min/max

GraphDraw::GraphDraw(QWidget *parent):
    QGraphicsView(parent),
    _graphEditor(dynamic_cast<GraphEditor *>(parent)),
    _zoomScale(1.0),
    _selectionState(0)
{
    //setup scene
    this->setScene(new QGraphicsScene(QRectF(QPointF(), GraphDrawCanvasSize), this));
    //required: BspTreeIndex is too smart for its own good, connections will not render properly
    this->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
    this->scene()->setBackgroundBrush(QColor(GraphDrawBackgroundColor));
    this->setDragMode(QGraphicsView::RubberBandDrag);
    this->ensureVisible(QRectF()); //set scrolls to 0, 0 position

    //set high quality rendering
    this->setRenderHint(QPainter::Antialiasing);
    this->setRenderHint(QPainter::HighQualityAntialiasing);
    this->setRenderHint(QPainter::SmoothPixmapTransform);

    //init settings
    assert(this->getGraphEditor() != nullptr);
    this->setAcceptDrops(true);
    this->setZoomScale(1.0);
    this->clearSelectionState();
    this->setFocusPolicy(Qt::ClickFocus);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
        this, SLOT(handleCustomContextMenuRequested(const QPoint &)));
    connect(this, SIGNAL(modifyProperties(QObject *)),
        getObjectMap()["propertiesPanel"], SLOT(handleGraphModifyProperties(QObject *)));
    connect(this->scene(), SIGNAL(selectionChanged(void)), this, SLOT(updateEnabledActions(void)));

    //debug view - connect and initialize
    connect(getActionMap()["showGraphConnectionPoints"], SIGNAL(triggered(void)),
        this, SLOT(handleGraphDebugViewChange(void)));
    connect(getActionMap()["showGraphBoundingBoxes"], SIGNAL(triggered(void)),
        this, SLOT(handleGraphDebugViewChange(void)));
    this->handleGraphDebugViewChange();
}

void GraphDraw::handleGraphDebugViewChange(void)
{
    _graphConnectionPoints.reset();
    if (getActionMap()["showGraphConnectionPoints"]->isChecked())
    {
        _graphConnectionPoints.reset(new QGraphicsPixmapItem());
        this->scene()->addItem(_graphConnectionPoints.get());
    }

    _graphBoundingBoxes.reset();
    if (getActionMap()["showGraphBoundingBoxes"]->isChecked())
    {
        _graphBoundingBoxes.reset(new QGraphicsPixmapItem());
        this->scene()->addItem(_graphBoundingBoxes.get());
    }

    if (not this->isVisible()) return;
    this->render();
}

void GraphDraw::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/json/pothos_block") and
        not event->mimeData()->data("text/json/pothos_block").isEmpty())
    {
        event->acceptProposedAction();
    }
    else QGraphicsView::dragEnterEvent(event);
}

void GraphDraw::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("text/json/pothos_block") and
        not event->mimeData()->data("text/json/pothos_block").isEmpty())
    {
        event->acceptProposedAction();
    }
    else QGraphicsView::dragMoveEvent(event);
}

void GraphDraw::dropEvent(QDropEvent *event)
{
    const auto &byteArray = event->mimeData()->data("text/json/pothos_block");
    Poco::JSON::Parser p; p.parse(std::string(byteArray.constData(), byteArray.size()));
    const auto blockDesc = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();

    this->getGraphEditor()->handleAddBlock(blockDesc, this->mapToScene(event->pos()));
    event->acceptProposedAction();
}

void GraphDraw::setZoomScale(const qreal zoom)
{
    //stash the old coordinate
    auto mousePos = this->mapFromGlobal(QCursor::pos());
    const auto p0 = this->mapToScene(mousePos);

    //perform the zoom
    _zoomScale = zoom;
    this->setTransform(QTransform()); //clear
    this->scale(this->zoomScale(), this->zoomScale());
    this->render();

    //calculate the scroll movement
    const auto p1 = this->mapToScene(mousePos);
    if (this->rect().contains(mousePos))
    {
        this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value()-p1.x()+p0.x());
        this->verticalScrollBar()->setValue(this->verticalScrollBar()->value()-p1.y()+p0.y());
    }
}

void GraphDraw::showEvent(QShowEvent *event)
{
    this->updateEnabledActions();
    emit this->modifyProperties(nullptr); //resets the state of whoever is modding the properties
    this->render();
    QGraphicsView::showEvent(event);
}

void GraphDraw::keyPressEvent(QKeyEvent *event)
{
    //map a key-press to an action name
    QString name;
    switch(event->key())
    {
    case Qt::Key_Plus:     name = "increment"; break;
    case Qt::Key_Minus:    name = "decrement"; break;
    case Qt::Key_Return:   name = "objectProperties"; break;
    case Qt::Key_E:        name = "enable"; break;
    case Qt::Key_D:        name = "disable"; break;
    case Qt::Key_R:        name = "reeval"; break;
    case Qt::Key_Left:     name = "rotateLeft"; break;
    case Qt::Key_Right:    name = "rotateRight"; break;
    }

    //default action is taken when nothing is selected
    //or a graph widget has the focus or unmatched key.
    if (
        this->getObjectsSelected().isEmpty() or
        this->graphWidgetHasFocus() or name.isEmpty())
    {
        QGraphicsView::keyPressEvent(event);
    }

    //otherwise trigger the action given by name
    else
    {
        getActionMap()[name]->activate(QAction::Trigger);
    }
}

void GraphDraw::updateEnabledActions(void)
{
    auto selectedObjsNoC = this->getObjectsSelected(~GRAPH_CONNECTION);
    const bool selectedNoC = not selectedObjsNoC.empty();

    auto selectedObjs = this->getObjectsSelected();
    const bool selected = not selectedObjs.empty();

    auto selectedObjBlocks = this->getObjectsSelected(GRAPH_BLOCK);
    const bool selectedBlocks = not selectedObjBlocks.empty();

    getActionMap()["cut"]->setEnabled(selectedNoC);
    getActionMap()["copy"]->setEnabled(selectedNoC);
    getActionMap()["delete"]->setEnabled(selected);
    getActionMap()["rotateLeft"]->setEnabled(selectedNoC);
    getActionMap()["rotateRight"]->setEnabled(selectedNoC);
    getActionMap()["objectProperties"]->setEnabled(selected);
    getActionMap()["increment"]->setEnabled(selectedBlocks);
    getActionMap()["decrement"]->setEnabled(selectedBlocks);
    getActionMap()["enable"]->setEnabled(selected);
    getActionMap()["disable"]->setEnabled(selected);
    getActionMap()["reeval"]->setEnabled(selectedBlocks);
    getMenuMap()["setAffinityZone"]->setEnabled(selectedBlocks);

    //and enable/disable the actions in the move graph objects submenu
    for (auto child : getMenuMap()["moveGraphObjects"]->children())
    {
        auto action = dynamic_cast<QAction *>(child);
        if (action != nullptr) action->setEnabled(selectedNoC);
    }
}

void GraphDraw::render(void)
{
    if (not this->isVisible()) return;

    //pre-render to perform connection calculations
    const auto allObjs = this->getGraphObjects();
    for (auto obj : allObjs) obj->prerender();

    //clip the bounds
    for (auto obj : allObjs)
    {
        auto oldPos = obj->pos();
        oldPos.setX(std::min(std::max(oldPos.x(), 0.0), this->sceneRect().width()-obj->boundingRect().width()));
        oldPos.setY(std::min(std::max(oldPos.y(), 0.0), this->sceneRect().height()-obj->boundingRect().height()));
        obj->setPos(oldPos);
    }

    //optional debug pixmap overlay for connection points
    if (_graphConnectionPoints)
    {
        QPixmap pixmap(this->sceneRect().size().toSize());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        for (auto obj : allObjs)
        {
            painter.save();
            painter.translate(obj->pos());
            painter.rotate(obj->rotation());
            obj->renderConnectablePoints(painter);
            painter.restore();
        }
        _graphConnectionPoints->setPixmap(pixmap);
        _graphConnectionPoints->setZValue(std::numeric_limits<qreal>::max());
    }

    //optional debug pixmap overlay for bounding boxes
    if (_graphBoundingBoxes)
    {
        QPixmap pixmap(this->sceneRect().size().toSize());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        for (auto obj : allObjs)
        {
            painter.save();
            painter.translate(obj->pos());
            painter.rotate(obj->rotation());
            painter.setPen(QPen(Qt::red));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(obj->shape());
            painter.restore();
        }
        _graphBoundingBoxes->setPixmap(pixmap);
        _graphBoundingBoxes->setZValue(std::numeric_limits<qreal>::max());
    }

    //cause full redraw
    this->scene()->update();
    this->repaint();
}

void GraphDraw::handleCustomContextMenuRequested(const QPoint &pos)
{
    _lastContextMenuPos = this->mapToScene(pos);
    getMenuMap()["edit"]->exec(this->mapToGlobal(pos));
}
