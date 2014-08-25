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
 * custom QGraphicsProxyWidget to accept the mouseDoubleClickEvent
 **********************************************************************/
class MyGraphicsProxyWidget : public QGraphicsProxyWidget
{
public:
    MyGraphicsProxyWidget(QGraphicsItem *parent):
        QGraphicsProxyWidget(parent){}

    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
    {
        QGraphicsProxyWidget::mouseDoubleClickEvent(event);
    }
};

/***********************************************************************
 * GraphWidget private container
 **********************************************************************/
struct GraphWidget::Impl
{
    Impl(QGraphicsItem *parent):
        container(new GraphWidgetContainer()),
        graphicsWidget(new MyGraphicsProxyWidget(parent))
    {
        graphicsWidget->setWidget(container);
    }

    ~Impl(void)
    {
        graphicsWidget->setWidget(nullptr);
    }

    QPointer<GraphBlock> block;

    GraphWidgetContainer *container;
    MyGraphicsProxyWidget *graphicsWidget;
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
    connect(block, SIGNAL(IDChanged(const QString &)), this, SLOT(handleBlockIdChanged(const QString &)));
    this->handleBlockIdChanged(block->getId()); //init value
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

void GraphWidget::handleBlockIdChanged(const QString &id)
{
    _impl->container->setGripLabel(id);
}

QPainterPath GraphWidget::shape(void) const
{
    return _impl->graphicsWidget->shape();
}

void GraphWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    _impl->graphicsWidget->mouseDoubleClickEvent(event);
    QGraphicsObject::mouseDoubleClickEvent(event);
}

bool GraphWidget::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    //clicking the internal widget causes the same behaviour as clicking no widgets -- unselect everything
    //this also has the added bennefit of preventing a false move event if the internal widget has a drag
    if (watched == _impl->graphicsWidget and event->type() == QEvent::GraphicsSceneMousePress and
        _impl->container->widget() != nullptr and _impl->container->widget()->underMouse())
    {
        this->draw()->deselectAllObjs();
        const auto maxZValue = this->draw()->getMaxZValue();
        if (this->zValue() <= maxZValue) this->setZValue(maxZValue+1);
    }
    return GraphObject::sceneEventFilter(watched, event);
}

QVariant GraphWidget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        _impl->container->setSelected(this->isSelected());
    }

    return QGraphicsItem::itemChange(change, value);
}

void GraphWidget::render(QPainter &)
{
    assert(_impl);

    //update display widget when not set
    _impl->block->prerender();
    auto graphWidget = _impl->block->getGraphWidget();
    _impl->container->setWidget(graphWidget);
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
    if (graphObj == nullptr) throw Pothos::NotFoundException("GraphWidget::deserialize()", "cant resolve block with ID: '"+blockId.toStdString()+"'");
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
