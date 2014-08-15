// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphWidget.hpp"
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
#include <QPainter>
#include <vector>
#include <iostream>
#include <cassert>

/***********************************************************************
 * A resizable container with a QSizeGrip handle
 **********************************************************************/
class MySizeGrip : public QSizeGrip
{
    Q_OBJECT
public:
    MySizeGrip(QWidget *parent):
        QSizeGrip(parent)
    {
        return;
    }

signals:
    void resized(void);

protected:
    void mousePressEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            _pressPos = this->pos();
        }
        QSizeGrip::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (_pressPos != this->pos()) emit this->resized();
        }
        QSizeGrip::mouseReleaseEvent(event);
    }

private:
    QPointF _pressPos;
};


class ResizableWidgetContainer : public QWidget
{
    Q_OBJECT
public:
    ResizableWidgetContainer(void):
        _layout(new QVBoxLayout(this)),
        _grip(new MySizeGrip(this)),
        _widget(nullptr)
    {
        this->setLayout(_layout);
        _layout->setContentsMargins(QMargins(3, 3, 3, 3));
        _layout->setSpacing(1);
        _layout->addWidget(_grip, 0, Qt::AlignBottom | Qt::AlignRight);
        connect(_grip, SIGNAL(resized(void)), this, SIGNAL(resized(void)));
        _grip->hide();
    }

    ~ResizableWidgetContainer(void)
    {
        //remove this container as a parent to widget
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
        this->setShowGrip(false);
    }

    void setGripLabel(const QString &name)
    {
        _gripLabel = QStaticText(QString("<b><span color='black'>%1</span></b>").arg(name.toHtmlEscaped()));
    }

signals:
    void resized(void);

protected:
    void enterEvent(QEvent *event)
    {
        this->setShowGrip(true);
        QWidget::enterEvent(event);
    }

    void leaveEvent(QEvent *event)
    {
        this->setShowGrip(false);
        QWidget::leaveEvent(event);
    }

    void setShowGrip(const bool visible)
    {
        if (not _widget) return;
        _widget->show(); //needs visibility to calculate size

        //stash the relevant settings
        auto oldPolicy = _widget->sizePolicy();
        auto oldMinSize = _widget->minimumSize();

        //fix the sizes so the widget wont be hurt by adjustSize
        _widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        _widget->setMinimumSize(_widget->size());

        //change the visibility and adjust to the new overall size
        _grip->setVisible(visible);
        this->adjustSize();

        //restore settings to the widget
        _widget->setMinimumSize(oldMinSize);
        _widget->setSizePolicy(oldPolicy);
    }

    void paintEvent(QPaintEvent *event)
    {
        QWidget::paintEvent(event);
        if (not _widget) return;
        if (not _grip->isVisible()) return;

        QPainter painter(this);
        auto panelH = this->height() - _widget->height();
        painter.drawStaticText(QPointF((this->width()-_gripLabel.size().width())/2.,
            _widget->height()+(panelH-_gripLabel.size().height())/2.), _gripLabel);
        painter.end();
    }

private:
    QStaticText _gripLabel;
    QVBoxLayout *_layout;
    QSizeGrip *_grip;
    QPointer<QWidget> _widget;
};

/***********************************************************************
 * GraphWidget private container
 **********************************************************************/
struct GraphWidget::Impl
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
    auto displayWidget = _impl->block->getDisplayWidget();
    _impl->container->setWidget(displayWidget);

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

#include "GraphWidget.moc"
