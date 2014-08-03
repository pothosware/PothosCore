// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphObject.hpp"
#include "GraphEditor/Constants.hpp"
#include <QPainter>
#include <cassert>
#include <iostream>

struct GraphObject::Impl
{
    Impl(void):
        deleteFlag(false),
        selected(false),
        zIndex(0),
        rotation(0)
    {
        return;
    }
    QString id;
    QPointF position;
    bool deleteFlag;
    bool selected;
    int zIndex;
    int rotation;
};

GraphObject::GraphObject(QObject *parent):
    QObject(parent),
    _impl(new Impl())
{
    return;
}

GraphObject::~GraphObject(void)
{
    return;
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

void GraphObject::setSelected(const bool on)
{
    assert(_impl);
    _impl->selected = on;
}

bool GraphObject::getSelected(void) const
{
    assert(_impl);
    return _impl->selected;
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
    return QRectF(this->getPosition(), this->getPosition());
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

void GraphObject::setZIndex(const int index)
{
    assert(_impl);
    _impl->zIndex = index;
}

int GraphObject::getZIndex(void) const
{
    assert(_impl);
    return _impl->zIndex;
}

void GraphObject::setPosition(const QPointF &pos)
{
    assert(_impl);
    _impl->position = pos;
}

const QPointF &GraphObject::getPosition(void) const
{
    assert(_impl);
    return _impl->position;
}

void GraphObject::move(const QPointF &delta)
{
    assert(_impl);
    _impl->position += delta;
}

void GraphObject::setRotation(const int degree)
{
    assert(_impl);
    _impl->rotation = degree;
}

int GraphObject::getRotation(void) const
{
    assert(_impl);
    return _impl->rotation;
}

void GraphObject::rotateLeft(void)
{
    assert(_impl);
    _impl->rotation = (_impl->rotation + 270) % 360;
    assert((_impl->rotation % 90) == 0);
    assert((_impl->rotation >= 0) and (_impl->rotation <= 270));
}

void GraphObject::rotateRight(void)
{
    assert(_impl);
    _impl->rotation = (_impl->rotation + 90) % 360;
    assert((_impl->rotation % 90) == 0);
    assert((_impl->rotation >= 0) and (_impl->rotation <= 270));
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
    obj->set("zIndex", int(this->getZIndex()));
    obj->set("positionX", int(this->getPosition().x()));
    obj->set("positionY", int(this->getPosition().y()));
    obj->set("rotation", int(this->getRotation()));
    obj->set("selected", this->getSelected());
    return obj;
}

void GraphObject::deserialize(Poco::JSON::Object::Ptr obj)
{
    this->setId(QString::fromStdString(obj->getValue<std::string>("id")));
    this->setZIndex(obj->getValue<int>("zIndex"));
    this->setPosition(QPointF(obj->getValue<int>("positionX"), obj->getValue<int>("positionY")));
    this->setRotation(obj->getValue<int>("rotation"));
    this->setSelected(obj->getValue<bool>("selected"));
}
