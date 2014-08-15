// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphWidget.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphObjects/GraphWidgetContainer.hpp"
#include "GraphEditor/Constants.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <Pothos/Exception.hpp>
#include <QGraphicsProxyWidget>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <vector>
#include <iostream>
#include <cassert>

/***********************************************************************
 * GraphWidget private container
 **********************************************************************/
struct GraphWidget::Impl
{
    Impl(QGraphicsItem *parent):
        changed(true),
        container(new GraphWidgetContainer()),
        graphicsWidget(new QGraphicsProxyWidget(parent))
    {
        graphicsWidget->setWidget(container);
    }

    ~Impl(void)
    {
        graphicsWidget->setWidget(nullptr);
    }

    bool changed;

    QPointer<GraphBlock> block;

    QRectF mainRect;

    GraphWidgetContainer *container;
    QGraphicsProxyWidget *graphicsWidget;
};

/***********************************************************************
 * GraphWidget Implementation
 **********************************************************************/
GraphWidget::GraphWidget(QObject *parent):
    GraphObject(parent),
    _impl(new Impl(this))
{
    this->setFlag(QGraphicsItem::ItemIsMovable);
    _impl->graphicsWidget->installSceneEventFilter(this);
    connect(_impl->container, SIGNAL(resized(void)), this, SLOT(handleWidgetResized(void)));
}

GraphWidget::~GraphWidget(void)
{
    return;
}

void GraphWidget::setGraphBlock(GraphBlock *block)
{
    //can only set block once
    assert(_impl);
    assert(not _impl->block);

    _impl->block = block;
    connect(block, SIGNAL(destroyed(QObject *)), this, SLOT(handleBlockDestroyed(QObject *)));
}

GraphBlock *GraphWidget::getGraphBlock(void) const
{
    return _impl->block;
}

void GraphWidget::handleBlockDestroyed(QObject *)
{
    //an endpoint was destroyed, schedule for deletion
    //however, the top level code should handle this deletion
    _impl->container->setWidget(nullptr);
    this->flagForDelete();
}

void GraphWidget::handleWidgetResized(void)
{
    auto editor = this->draw()->getGraphEditor();
    editor->handleStateChange(GraphState("transform-scale", tr("Resize %1").arg(this->getId())));
}

bool GraphWidget::isPointing(const QRectF &rect) const
{
    return _impl->mainRect.intersects(rect);
}

QPainterPath GraphWidget::shape(void) const
{
    QPainterPath path;
    path.addRect(_impl->mainRect);
    return path;
}

bool GraphWidget::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    //clicking the internal widget causes the same behaviour as clicking no widgets -- unselect everything
    //this also has the added bennefit of preventing a false move event if the internal widget has a drag
    if (watched == _impl->graphicsWidget and event->type() == QEvent::GraphicsSceneMousePress)
    {
        this->draw()->deselectAllObjs();
        const auto maxZValue = this->draw()->getMaxZValue();
        if (this->zValue() <= maxZValue) this->setZValue(maxZValue+1);
    }
    return GraphObject::sceneEventFilter(watched, event);
}

void GraphWidget::render(QPainter &painter)
{
    assert(_impl);

    //render text
    if (_impl->changed)
    {
        _impl->changed = false;
        _impl->container->setGripLabel(_impl->block->getId());
    }

    //update display widget when not set
    auto graphWidget = _impl->block->getGraphWidget();
    _impl->container->setWidget(graphWidget);

    //calculate the bounds and draw the highlight box
    const auto widgetSize = _impl->graphicsWidget->size();
    painter.setPen(isSelected()?QColor(GraphObjectHighlightPenColor):Qt::transparent);
    painter.setBrush(QBrush(Qt::transparent));
    _impl->mainRect = QRectF(_impl->graphicsWidget->pos(), widgetSize);
    painter.drawRect(_impl->mainRect);
}

Poco::JSON::Object::Ptr GraphWidget::serialize(void) const
{
    auto obj = GraphObject::serialize();
    obj->set("what", std::string("Widget"));
    obj->set("blockId", _impl->block->getId().toStdString());
    obj->set("width", _impl->graphicsWidget->size().width());
    obj->set("height", _impl->graphicsWidget->size().height());
    return obj;
}

void GraphWidget::deserialize(Poco::JSON::Object::Ptr obj)
{
    auto editor = this->draw()->getGraphEditor();

    //locate the associated block
    auto blockId = QString::fromStdString(obj->getValue<std::string>("blockId"));
    auto graphObj = editor->getObjectById(blockId, GRAPH_BLOCK);
    if (graphObj == nullptr) throw Pothos::Exception("GraphWidget::deserialize()", "cant resolve block with ID: '"+blockId.toStdString()+"'");
    auto graphBlock = dynamic_cast<GraphBlock *>(graphObj);
    assert(graphBlock != nullptr);
    this->setGraphBlock(graphBlock);

    if (obj->has("width") and obj->has("height"))
    {
        _impl->graphicsWidget->resize(
            obj->getValue<int>("width"),
            obj->getValue<int>("height"));
    }

    GraphObject::deserialize(obj);
}
