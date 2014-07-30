// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //action and object map
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphEditor/Constants.hpp"
#include <Poco/JSON/Parser.h>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QImage>
#include <QAction>
#include <QChildEvent>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <iostream>
#include <cassert>

GraphDraw::GraphDraw(QWidget *parent):
    QWidget(parent),
    _graphEditor(dynamic_cast<GraphEditor *>(parent)),
    _zoomScale(1.0),
    _mouseLeftDown(false),
    _showGraphConnectionPoints(false),
    _showGraphBoundingBoxes(false)
{
    assert(this->getGraphEditor() != nullptr);
    this->setAcceptDrops(true);

    this->setZoomScale(1.0);

    this->setFocusPolicy(Qt::ClickFocus);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
        this, SLOT(handleCustomContextMenuRequested(const QPoint &)));
    connect(this, SIGNAL(stateChanged(const GraphState &)),
        parent, SLOT(handleStateChange(const GraphState &)));
    connect(this, SIGNAL(modifyProperties(GraphObject *)),
        getObjectMap()["propertiesPanel"], SLOT(handleGraphModifyProperties(GraphObject *)));

    //debug view - connect and initialize
    connect(getActionMap()["showGraphConnectionPoints"], SIGNAL(triggered(void)),
        this, SLOT(handleGraphDebugViewChange(void)));
    connect(getActionMap()["showGraphBoundingBoxes"], SIGNAL(triggered(void)),
        this, SLOT(handleGraphDebugViewChange(void)));
    this->handleGraphDebugViewChange();
}

void GraphDraw::handleGraphDebugViewChange(void)
{
    _showGraphConnectionPoints = getActionMap()["showGraphConnectionPoints"]->isChecked();
    _showGraphBoundingBoxes = getActionMap()["showGraphBoundingBoxes"]->isChecked();
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
    QWidget::dragEnterEvent(event);
}

void GraphDraw::dropEvent(QDropEvent *event)
{
    const auto &byteArray = event->mimeData()->data("text/json/pothos_block");
    Poco::JSON::Parser p; p.parse(std::string(byteArray.constData(), byteArray.size()));
    const auto blockDesc = p.getHandler()->asVar().extract<Poco::JSON::Object::Ptr>();

    this->getGraphEditor()->handleAddBlock(blockDesc, event->pos());
    QWidget::dropEvent(event);
}

void GraphDraw::setZoomScale(const qreal zoom)
{
    _zoomScale = zoom;
    QSizeF newSize(GraphDrawCanvasSize*this->zoomScale());
    _image = QImage(newSize.width(), newSize.height(), QImage::Format_ARGB32);
    this->resize(_image.size());
    this->render();
}

void GraphDraw::setupCanvas(void)
{
    QSizeF newSize(GraphDrawCanvasSize*this->zoomScale());
    _image = QImage(newSize.width(), newSize.height(), QImage::Format_ARGB32);
    this->resize(_image.size());
    this->render();
}

void GraphDraw::paintEvent(QPaintEvent *event)
{
    QRectF target(QPointF(), this->size());
    QRectF source(QPointF(), _image.size());
    QPainter(this).drawImage(target, _image, source);

    QWidget::paintEvent(event);
}

void GraphDraw::showEvent(QShowEvent *event)
{
    emit this->modifyProperties(nullptr); //resets the state of whoever is modding the properties
    this->setupCanvas(); //size could have changed, resize to fit
    this->render();
    QWidget::showEvent(event);
}

void GraphDraw::updateEnabledActions(void)
{
    auto selectedObjsNoC = this->getObjectsSelected(~GRAPH_CONNECTION);
    const bool selectedNoC = not selectedObjsNoC.empty();

    auto selectedObjs = this->getObjectsSelected();
    const bool selected = not selectedObjs.empty();

    getActionMap()["cut"]->setEnabled(selectedNoC);
    getActionMap()["copy"]->setEnabled(selectedNoC);
    getActionMap()["delete"]->setEnabled(selected);
    getActionMap()["rotateLeft"]->setEnabled(selectedNoC);
    getActionMap()["rotateRight"]->setEnabled(selectedNoC);
    getActionMap()["properties"]->setEnabled(selected);

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

    //its convenient to always update this here
    this->updateEnabledActions();

    //draw background
    _image.fill(QColor(GraphDrawBackgroundColor));

    //setup painter
    QPainter painter(&_image);

    //pre-render to perform connection calculations
    const auto allObjs = this->getGraphObjects();
    for (auto obj : allObjs) obj->prerender();

    //set high quality rendering after drawing the background
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    //clip the bounds
    for (auto obj : allObjs)
    {
        auto oldPos = obj->getPosition();
        oldPos.setX(std::min(std::max(oldPos.x(), 0.0), QSizeF(this->size()).width()));
        oldPos.setY(std::min(std::max(oldPos.y(), 0.0), QSizeF(this->size()).height()));
        obj->setPosition(oldPos);
    }

    //render objects
    for (auto obj : allObjs)
    {
        //draw the object
        painter.save();
        painter.scale(this->zoomScale(), this->zoomScale());
        obj->render(painter);
        painter.restore();

        //draw connection points (for debug)
        if (_showGraphConnectionPoints)
        {
            painter.save();
            painter.scale(this->zoomScale(), this->zoomScale());
            obj->renderConnectablePoints(painter);
            painter.restore();
        }

        //draw bounding boxes (for debug)
        if (_showGraphBoundingBoxes)
        {
            painter.save();
            painter.scale(this->zoomScale(), this->zoomScale());
            auto boundingRect = obj->getBoundingRect();
            painter.setPen(QPen(QColor("red")));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(boundingRect);
            painter.restore();
        }
    }

    if (_selectionState == "highlight")
    {
        QColor hc(GraphDrawHighlightColor);
        hc.setAlpha(GraphDrawHighlightAlpha);
        painter.setBrush(QBrush(hc));
        painter.setPen(Qt::NoPen);
        painter.drawRect(QRectF(_mouseLeftDownFirstPoint, _mouseLeftDownLastPoint));
    }

    painter.end();
    this->repaint();
}

void GraphDraw::handleCustomContextMenuRequested(const QPoint &pos)
{
    auto menu = new QMenu(this);
    menu->addAction(getActionMap()["cut"]);
    menu->addAction(getActionMap()["copy"]);
    menu->addAction(getActionMap()["paste"]);
    menu->addAction(getActionMap()["delete"]);
    menu->addSeparator();
    menu->addAction(getActionMap()["selectAll"]);
    menu->addSeparator();
    menu->addAction(getActionMap()["find"]);
    menu->addSeparator();
    menu->addAction(getActionMap()["createGraphPage"]);
    menu->addAction(getActionMap()["renameGraphPage"]);
    menu->addAction(getActionMap()["deleteGraphPage"]);
    menu->addMenu(getMenuMap()["moveGraphObjects"]);
    menu->addMenu(getMenuMap()["setAffinityZone"]);
    menu->addSeparator();
    menu->addAction(getActionMap()["createInputBreaker"]);
    menu->addAction(getActionMap()["createOutputBreaker"]);
    menu->addSeparator();
    menu->addAction(getActionMap()["rotateLeft"]);
    menu->addAction(getActionMap()["rotateRight"]);

    _lastContextMenuPos = pos;
    menu->exec(this->mapToGlobal(pos));
    delete menu;
}

QWidget *makeGraphDraw(QWidget *parent)
{
    return new GraphDraw(parent);
};
