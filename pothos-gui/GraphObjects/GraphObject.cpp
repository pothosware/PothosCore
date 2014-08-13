// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphObject.hpp"
#include "GraphEditor/Constants.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include <QGraphicsView>
#include <QPainter>
#include <cassert>
#include <iostream>

struct GraphObject::Impl
{
    Impl(void):
        deleteFlag(false)
    {
        return;
    }
    QString id;
    bool deleteFlag;
};

GraphObject::GraphObject(QObject *parent):
    QGraphicsObject(),
    _impl(new Impl())
{
    auto view = dynamic_cast<QGraphicsView *>(parent);
    assert(view != nullptr);
    view->scene()->addItem(this);
    this->setFlag(QGraphicsItem::ItemIsSelectable);
}

GraphObject::~GraphObject(void)
{
    return;
}

GraphDraw *GraphObject::draw(void) const
{
    auto draw = dynamic_cast<GraphDraw *>(this->scene()->views().at(0));
    assert(draw != nullptr);
    return draw;
}

void GraphObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    this->render(*painter);
}

void GraphObject::setId(const QString &id)
{
    assert(_impl);
    _impl->id = id;
}

const QString &GraphObject::getId(void) const
{
    assert(_impl);
    return _impl->id;
}

QRectF GraphObject::boundingRect(void) const
{
    return this->shape().boundingRect();
}

QPainterPath GraphObject::shape(void) const
{
    return QGraphicsItem::shape();
}

void GraphObject::prerender(void)
{
    QImage i0(1, 1, QImage::Format_ARGB32);
    QPainter p0(&i0);
    this->render(p0);
}

void GraphObject::render(QPainter &)
{
    return;
}

void GraphObject::rotateLeft(void)
{
    this->setRotation(int(this->rotation() + 270) % 360);
}

void GraphObject::rotateRight(void)
{
    this->setRotation(int(this->rotation() + 90) % 360);
}

std::vector<GraphConnectableKey> GraphObject::getConnectableKeys(void) const
{
    return std::vector<GraphConnectableKey>();
}

GraphConnectableKey GraphObject::isPointingToConnectable(const QPointF &) const
{
    return GraphConnectableKey();
}

GraphConnectableAttrs GraphObject::getConnectableAttrs(const GraphConnectableKey &) const
{
    return GraphConnectableAttrs();
}

bool GraphObject::isFlaggedForDelete(void) const
{
    assert(_impl);
    return _impl->deleteFlag;
}

void GraphObject::flagForDelete(void)
{
    assert(_impl);
    _impl->deleteFlag = true;
    emit this->deleteLater();
}

void GraphObject::renderConnectablePoints(QPainter &painter)
{
    for (const auto &key : this->getConnectableKeys())
    {
        const auto attrs = this->getConnectableAttrs(key);

        //draw circle
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(GraphObjectConnPointColor)));
        painter.drawEllipse(attrs.point, GraphObjectConnPointRadius, GraphObjectConnPointRadius);

        //draw vector
        painter.setPen(QPen(QColor(GraphObjectConnLineColor)));
        QTransform trans; trans.rotate(attrs.rotation-this->rotation());
        painter.drawLine(attrs.point, attrs.point + trans.map(QPointF(GraphObjectConnLineLength, 0)));
    }
}

Poco::JSON::Object::Ptr GraphObject::serialize(void) const
{
    Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
    obj->set("id", this->getId().toStdString());
    obj->set("zValue", double(this->zValue()));
    obj->set("positionX", double(this->pos().x()));
    obj->set("positionY", double(this->pos().y()));
    obj->set("rotation", double(this->rotation()));
    obj->set("selected", this->isSelected());
    return obj;
}

void GraphObject::deserialize(Poco::JSON::Object::Ptr obj)
{
    this->setId(QString::fromStdString(obj->getValue<std::string>("id")));
    this->setZValue(obj->optValue<double>("zValue", 0.0));
    this->setPos(QPointF(obj->optValue<double>("positionX", 0.0), obj->optValue<double>("positionY", 0.0)));
    this->setRotation(obj->optValue<double>("rotation", 0.0));
    this->setSelected(obj->optValue<bool>("selected", false));
}
