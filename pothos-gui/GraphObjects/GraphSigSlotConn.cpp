// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphSigSlotConn.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/Constants.hpp"
#include <Pothos/Exception.hpp>
#include <QPainter>
#include <QPolygonF>
#include <iostream>
#include <cassert>

struct GraphSigSlotConn::Impl
{
    GraphConnectionEndpoint slotEp;
    GraphConnectionEndpoint signalEp;

    //TODO line
    QPolygonF arrowHead;
};

GraphSigSlotConn::GraphSigSlotConn(QObject *parent):
    GraphObject(parent),
    _impl(new Impl())
{
    return;
}

void GraphSigSlotConn::setupEndpoint(const GraphConnectionEndpoint &ep)
{
    assert(_impl);
    if (ep.getKey().direction == GRAPH_CONN_SLOT) _impl->slotEp = ep;
    if (ep.getKey().direction == GRAPH_CONN_SIGNAL) _impl->signalEp = ep;
    connect(ep.getObj(), SIGNAL(destroyed(QObject *)), this, SLOT(handleEndPointDestroyed(QObject *)));
}

const GraphConnectionEndpoint &GraphSigSlotConn::getSignalEndpoint(void) const
{
    assert(_impl);
    return _impl->signalEp;
}

const GraphConnectionEndpoint &GraphSigSlotConn::getSlotEndpoint(void) const
{
    assert(_impl);
    return _impl->slotEp;
}

void GraphSigSlotConn::handleEndPointDestroyed(QObject *)
{
    //an endpoint was destroyed, schedule for deletion
    //however, the top level code should handle this deletion
    this->flagForDelete();
}

bool GraphSigSlotConn::isPointing(const QRectF &rect) const
{
    //TODO line
    //TODO text -- so we can click select it

    //check arrow head
    return not _impl->arrowHead.intersected(rect).isEmpty();
}

QRectF GraphSigSlotConn::getBoundingRect(void) const
{
    QVector<QPointF> points;//TODO = _impl->points;
    const auto arrowRect = _impl->arrowHead.boundingRect();
    points.push_back(arrowRect.topLeft());
    points.push_back(arrowRect.topRight());
    points.push_back(arrowRect.bottomRight());
    points.push_back(arrowRect.bottomLeft());
    return QPolygonF(points).boundingRect();
}

void GraphSigSlotConn::render(QPainter &painter)
{
    assert(_impl);
    //dont draw connections with missing endpoints
    if (not _impl->signalEp.isValid()) return;
    if (not _impl->slotEp.isValid()) return;

    //query the connectable info
    const auto signalAttrs = _impl->signalEp.getConnectableAttrs();
    const auto slotAttrs = _impl->slotEp.getConnectableAttrs();

    //make the minimal output protrusion
    const auto op0 = signalAttrs.point;
    QTransform otrans; otrans.rotate(signalAttrs.rotation);
    const auto op1 = signalAttrs.point + otrans.map(QPointF(GraphConnectionMinPling, 0));

    //make the minimal input protrusion
    const auto ip0 = slotAttrs.point;
    QTransform itrans; itrans.rotate(slotAttrs.rotation + 180);
    const auto ip1 = slotAttrs.point + itrans.map(QPointF(GraphConnectionMinPling+GraphConnectionArrowLen, 0));

    //such TODO


    //draw the painter path
    QColor color(getSelected()?GraphConnectionHighlightColor:GraphConnectionDefaultColor);
    painter.setBrush(Qt::NoBrush);
    QPen pen(color);
    pen.setWidthF(GraphConnectionGirth);
    painter.setPen(pen);
    //painter.drawPath(path);
    //_impl->points = points;

    //create arrow head
    QTransform trans0; trans0.rotate(slotAttrs.rotation + GraphConnectionArrowAngle);
    QTransform trans1; trans1.rotate(slotAttrs.rotation - GraphConnectionArrowAngle);
    const auto p0 = trans0.map(QPointF(-GraphConnectionArrowLen, 0));
    const auto p1 = trans1.map(QPointF(-GraphConnectionArrowLen, 0));
    QPolygonF arrowHead;
    arrowHead << ip0 << (ip0+p0) << (ip0+p1);
    painter.setBrush(QBrush(color));
    painter.drawPolygon(arrowHead);
    _impl->arrowHead = arrowHead;
}

Poco::JSON::Object::Ptr GraphSigSlotConn::serialize(void) const
{
    auto obj = GraphObject::serialize();
    assert(this->getSignalEndpoint().isValid());
    assert(this->getSlotEndpoint().isValid());
    obj->set("what", std::string("SigSlotConn"));
    obj->set("signalId", this->getSignalEndpoint().getObj()->getId().toStdString());
    obj->set("slotId", this->getSlotEndpoint().getObj()->getId().toStdString());
    obj->set("signalKey", this->getSignalEndpoint().getKey().id.toStdString());
    obj->set("slotKey", this->getSlotEndpoint().getKey().id.toStdString());
    return obj;
}

void GraphSigSlotConn::deserialize(Poco::JSON::Object::Ptr obj)
{
    auto signalId = QString::fromStdString(obj->getValue<std::string>("signalId"));
    auto slotId = QString::fromStdString(obj->getValue<std::string>("slotId"));
    auto signalKey = QString::fromStdString(obj->getValue<std::string>("signalKey"));
    auto slotKey = QString::fromStdString(obj->getValue<std::string>("slotKey"));

    auto draw = dynamic_cast<GraphDraw *>(this->parent());
    assert(draw != nullptr);

    //resolve IO objects by id
    QPointer<GraphObject> slotObj = nullptr;
    QPointer<GraphObject> signalObj = nullptr;
    for (const auto obj : draw->getGraphObjects(GRAPH_BLOCK | GRAPH_BREAKER))
    {
        if (obj->getId() == slotId) slotObj = obj;
        if (obj->getId() == signalId) signalObj = obj;
    }

    if (slotObj.isNull()) throw Pothos::Exception("GraphConnection::deserialize()", "cant resolve object with ID: '"+slotId.toStdString()+"'");
    if (signalObj.isNull()) throw Pothos::Exception("GraphConnection::deserialize()", "cant resolve object with ID: '"+signalId.toStdString()+"'");

    this->setupEndpoint(GraphConnectionEndpoint(slotObj, GraphConnectableKey(slotKey, GRAPH_CONN_SLOT)));
    this->setupEndpoint(GraphConnectionEndpoint(signalObj, GraphConnectableKey(signalKey, GRAPH_CONN_SIGNAL)));

    assert(this->getSlotEndpoint().isValid());
    assert(this->getSignalEndpoint().isValid());

    GraphObject::deserialize(obj);
}
