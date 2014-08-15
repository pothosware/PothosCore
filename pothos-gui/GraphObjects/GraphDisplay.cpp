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
#include <QSizeGrip>
#include <QVBoxLayout>
#include <QStaticText>
#include <vector>
#include <iostream>
#include <cassert>

/***********************************************************************
 * A resizable container with a QSizeGrip handle
 **********************************************************************/
class ResizableWidgetContainer : public QWidget
{
    Q_OBJECT
public:
    ResizableWidgetContainer(void):
        _layout(new QVBoxLayout(this)),
        _grip(new QSizeGrip(this)),
        _widget(nullptr)
    {
        this->setLayout(_layout);

        // To remove any space between the borders and the QSizeGrip...
        _layout->setContentsMargins(QMargins());

        // and between the other widget and the QSizeGrip
        _layout->setSpacing(0);

        // The QSizeGrip position (here Bottom Right Corner)
        _layout->addWidget(_grip, 0, Qt::AlignBottom | Qt::AlignRight);
    }

    ~ResizableWidgetContainer(void)
    {
        this->setWidget(nullptr);
    }

    void setWidget(QWidget *widget)
    {
        //no change, just return
        if (_widget == widget) return;

        //remove old widget, dont delete it
        if (_widget)
        {
            //we dont own the widget, dont delete it
            _layout->removeWidget(_widget);
            _widget->setParent(nullptr);
        }

        //stash new widget and add to layout
        _widget = widget;
        if (_widget) _layout->insertWidget(0, _widget);
    }

private:
    QVBoxLayout *_layout;
    QSizeGrip *_grip;
    QPointer<QWidget> _widget;
};

/***********************************************************************
 * GraphDisplay private container
 **********************************************************************/
struct GraphDisplay::Impl
{
    Impl(QGraphicsItem *parent):
        changed(true),
        container(new ResizableWidgetContainer()),
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

    ResizableWidgetContainer *container;
    QGraphicsProxyWidget *graphicsWidget;
};

/***********************************************************************
 * GraphDisplay Implementation
 **********************************************************************/
GraphDisplay::GraphDisplay(QObject *parent):
    GraphObject(parent),
    _impl(new Impl(this))
{
    this->setFlag(QGraphicsItem::ItemIsMovable);
    _impl->graphicsWidget->installSceneEventFilter(this);
}

GraphDisplay::~GraphDisplay(void)
{
    return;
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
    _impl->container->setWidget(nullptr);
    this->flagForDelete();
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
    if (watched == _impl->graphicsWidget and event->type() == QEvent::GraphicsSceneMousePress)
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
    _impl->container->setWidget(displayWidget);

    //calculate the bounds and draw the highlight box
    const auto widgetSize = _impl->graphicsWidget->size();
    painter.setPen(isSelected()?QColor(GraphObjectHighlightPenColor):Qt::transparent);
    painter.setBrush(QBrush(Qt::transparent));
    _impl->mainRect = QRectF(_impl->graphicsWidget->pos(), widgetSize);
    painter.drawRect(_impl->mainRect);
}

Poco::JSON::Object::Ptr GraphDisplay::serialize(void) const
{
    auto obj = GraphObject::serialize();
    obj->set("what", std::string("Display"));
    obj->set("blockId", _impl->block->getId().toStdString());
    obj->set("width", _impl->graphicsWidget->size().width());
    obj->set("height", _impl->graphicsWidget->size().height());
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

    if (obj->has("width") and obj->has("height"))
    {
        _impl->graphicsWidget->resize(
            obj->getValue<int>("width"),
            obj->getValue<int>("height"));
    }

    GraphObject::deserialize(obj);
}

#include "GraphDisplay.moc"
