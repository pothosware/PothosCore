// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphObject.hpp"
#include "GraphEditor/Constants.hpp"
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
    QObject(parent),
    QGraphicsItem(),
    _impl(new Impl())
{
    auto view = dynamic_cast<QGraphicsView *>(parent);
    assert(view != nullptr);
    view->scene()->addItem(this);
    this->setFlag(QGraphicsItem::ItemIsMovable);
    this->setFlag(QGraphicsItem::ItemIsSelectable);
}

GraphObject::~GraphObject(void)
{
    return;
}

QRectF GraphObject::boundingRect(void) const
{
    return this->getBoundingRect();
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

bool GraphObject::isPointing(const QPointF &point) const
{
    return this->isPointing(QRectF(point-QPointF(1, 1), point+QPointF(1, 1)));
}

bool GraphObject::isPointing(const QRectF &) const
{
    return false;
}

QRectF GraphObject::getBoundingRect(void) const
{
    return QRectF(this->pos(), this->pos());
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
        QTransform trans; trans.rotate(attrs.rotation);
        painter.drawLine(attrs.point, attrs.point + trans.map(QPointF(GraphObjectConnLineLength, 0)));
    }
}

Poco::JSON::Object::Ptr GraphObject::serialize(void) const
{
    Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
    obj->set("id", this->getId().toStdString());
    obj->set("zIndex", int(this->zValue()));
    obj->set("positionX", int(this->pos().x()));
    obj->set("positionY", int(this->pos().y()));
    obj->set("rotation", int(this->rotation()));
    obj->set("selected", this->isSelected());
    return obj;
}

void GraphObject::deserialize(Poco::JSON::Object::Ptr obj)
{
    this->setId(QString::fromStdString(obj->getValue<std::string>("id")));
    this->setZValue(obj->getValue<int>("zIndex"));
    this->setPos(QPointF(obj->getValue<int>("positionX"), obj->getValue<int>("positionY")));
    this->setRotation(obj->getValue<int>("rotation"));
    this->setSelected(obj->getValue<bool>("selected"));
}
