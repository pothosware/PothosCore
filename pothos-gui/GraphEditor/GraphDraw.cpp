// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphConnection.hpp"
#include "GraphEditor/Constants.hpp"
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
    _mouseLeftDown(false)
{
    assert(this->getGraphEditor() != nullptr);
    this->setAcceptDrops(true);

    this->setZoomScale(1.0);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
        this, SLOT(handleCustomContextMenuRequested(const QPoint &)));
    connect(this, SIGNAL(stateChanged(const GraphState &)),
        parent, SLOT(handleStateChange(const GraphState &)));
    connect(this, SIGNAL(modifyProperties(GraphObject *)),
        getWidgetMap()["propertiesPanel"], SLOT(handleGraphModifyProperties(GraphObject *)));
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
    this->getGraphEditor()->handleAddBlock(event->mimeData()->data("text/json/pothos_block"), event->pos());
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
    auto selectedObjsNoC = this->getObjectsSelected(false/*NC*/);
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
    //its convenient to always update this here
    this->updateEnabledActions();

    //setup painter
    QPainter painter(&_image);

    //pre-render to perform connection calculations
    const auto allObjs = this->getGraphObjects();
    for (auto obj : allObjs)
    {
        painter.save();
        obj->render(painter);
        painter.restore();
    }

    //draw background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(GraphDrawBackgroundColor)));
    painter.drawRect(QRect(QPoint(), _image.size()));

    //set high quality rendering after drawing the background
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    //render objects
    for (auto obj : allObjs)
    {
        //draw the object
        painter.save();
        painter.scale(this->zoomScale(), this->zoomScale());
        obj->render(painter);
        painter.restore();

        //draw connection points (for debug)
        /*
        painter.save();
        painter.scale(this->zoomScale(), this->zoomScale());
        obj->renderConnectablePoints(painter);
        painter.restore();
        //*/
    }

    if (_selectionState == "highlight")
    {
        QColor hc(GraphDrawHighlightColor);
        hc.setAlpha(GraphDrawHighlightAlpha);
        painter.setBrush(QBrush(hc));
        painter.setPen(Qt::NoPen);
        painter.drawRect(QRectF(_mouseLeftDownFirstPoint, _mouseLeftDownLastPoint));
    }

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
    menu->addAction(getActionMap()["createGraphPage"]);
    menu->addAction(getActionMap()["renameGraphPage"]);
    menu->addAction(getActionMap()["deleteGraphPage"]);
    menu->addMenu(getMenuMap()["moveGraphObjects"]);
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
