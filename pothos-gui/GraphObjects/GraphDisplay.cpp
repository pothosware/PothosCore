// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphDisplay.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphEditor/Constants.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <Pothos/Exception.hpp>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QStaticText>
#include <vector>
#include <iostream>
#include <cassert>

struct GraphDisplay::Impl
{
    Impl(void):
        changed(true)
    {
        return;
    }
    bool changed;

    QPointer<GraphBlock> block;

    QRectF mainRect;

    std::shared_ptr<QGraphicsProxyWidget> graphicsWidget;
};

GraphDisplay::GraphDisplay(QObject *parent):
    GraphObject(parent),
    _impl(new Impl())
{
    this->setFlag(QGraphicsItem::ItemIsMovable);
}

GraphDisplay::~GraphDisplay(void)
{
    //special destroy that causes the graphicsWidget to relinquish widget ownership
    //thats because the block and not the graphicsWidget really owns the widget
    this->handleWidgetDestroyed(nullptr);
}

void GraphDisplay::setGraphBlock(GraphBlock *block)
{
    //can only set block once
    assert(_impl);
    assert(not _impl->block);

    _impl->block = block;
    connect(block, SIGNAL(destroyed(QObject *)), this, SLOT(handleBlockDestroyed(QObject *)));
}

GraphBlock *GraphDisplay::getGraphBlock(void) const
{
    return _impl->block;
}

void GraphDisplay::handleBlockDestroyed(QObject *)
{
    //an endpoint was destroyed, schedule for deletion
    //however, the top level code should handle this deletion
    this->flagForDelete();
}

void GraphDisplay::handleWidgetDestroyed(QObject *)
{
    if (_impl->graphicsWidget)
    {
        _impl->graphicsWidget->setWidget(nullptr);
        _impl->graphicsWidget.reset();
    }
}

bool GraphDisplay::isPointing(const QRectF &rect) const
{
    return _impl->mainRect.intersects(rect);
}

QPainterPath GraphDisplay::shape(void) const
{
    QPainterPath path;
    path.addRect(_impl->mainRect);
    return path;
}

bool GraphDisplay::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    //clicking the internal widget causes the same behaviour as clicking no widgets -- unselect everything
    //this also has the added bennefit of preventing a false move event if the internal widget has a drag
    if (watched == _impl->graphicsWidget.get() and this->isSelected() and event->type() == QEvent::GraphicsSceneMousePress)
    {
        this->draw()->deselectAllObjs();
    }
    return GraphObject::sceneEventFilter(watched, event);
}

void GraphDisplay::render(QPainter &painter)
{
    assert(_impl);

    //render text
    if (_impl->changed)
    {
        _impl->changed = false;
    }

    //update display widget when not set
    auto displayWidget = _impl->block->getDisplayWidget();
    if (displayWidget and not _impl->graphicsWidget)
    {
        connect(displayWidget, SIGNAL(destroyed(QObject *)), this, SLOT(handleWidgetDestroyed(QObject *)));
        _impl->graphicsWidget.reset(new QGraphicsProxyWidget(this));
        _impl->graphicsWidget->setWidget(displayWidget);
        _impl->graphicsWidget->installSceneEventFilter(this);
    }

    if (_impl->graphicsWidget)
    {
        const auto widgetSize = _impl->graphicsWidget->size();

        auto pen = QPen(QColor(GraphObjectDefaultPenColor));
        pen.setWidthF(GraphObjectBorderWidth);
        painter.setPen(pen);
        if (isSelected()) painter.setPen(QColor(GraphObjectHighlightPenColor));
        painter.setBrush(QBrush(Qt::transparent));

        QRectF mainRect(_impl->graphicsWidget->pos(), widgetSize);
        mainRect.adjust(-10, -10, 10, 10);
        _impl->mainRect = mainRect;

        painter.drawRect(mainRect);
    }
}

Poco::JSON::Object::Ptr GraphDisplay::serialize(void) const
{
    auto obj = GraphObject::serialize();
    obj->set("what", std::string("Display"));
    obj->set("blockId", _impl->block->getId().toStdString());

    //TODO size

    return obj;
}

void GraphDisplay::deserialize(Poco::JSON::Object::Ptr obj)
{
    auto editor = this->draw()->getGraphEditor();

    //locate the associated block
    auto blockId = QString::fromStdString(obj->getValue<std::string>("blockId"));
    auto graphObj = editor->getObjectById(blockId, GRAPH_BLOCK);
    if (graphObj == nullptr) throw Pothos::Exception("GraphDisplay::deserialize()", "cant resolve block with ID: '"+blockId.toStdString()+"'");
    auto graphBlock = dynamic_cast<GraphBlock *>(graphObj);
    assert(graphBlock != nullptr);
    this->setGraphBlock(graphBlock);

    //TODO size

    GraphObject::deserialize(obj);
}
