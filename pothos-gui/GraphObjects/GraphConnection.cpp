// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphConnection.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/Constants.hpp"
#include <Pothos/Exception.hpp>
#include <QPainter>
#include <QPolygonF>
#include <iostream>
#include <cassert>

struct GraphConnection::Impl
{
    GraphConnectionEndpoint inputEp;
    GraphConnectionEndpoint outputEp;

    QVector<QPointF> points;
    QPolygonF arrowHead;
};

GraphConnection::GraphConnection(QObject *parent):
    GraphObject(parent),
    _impl(new Impl())
{
    return;
}

void GraphConnection::setupEndpoint(const GraphConnectionEndpoint &ep)
{
    assert(_impl);
    if (ep.getKey().direction == GRAPH_CONN_INPUT) _impl->inputEp = ep;
    if (ep.getKey().direction == GRAPH_CONN_OUTPUT) _impl->outputEp = ep;
    connect(ep.getObj(), SIGNAL(destroyed(QObject *)), this, SLOT(handleEndPointDestroyed(QObject *)));
}

const GraphConnectionEndpoint &GraphConnection::getOutputEndpoint(void) const
{
    assert(_impl);
    return _impl->outputEp;
}

const GraphConnectionEndpoint &GraphConnection::getInputEndpoint(void) const
{
    assert(_impl);
    return _impl->inputEp;
}

void GraphConnection::handleEndPointDestroyed(QObject *)
{
    //an endpoint was destroyed, schedule for deletion
    //however, the top level code should handle this deletion
    this->flagForDelete();
}

bool GraphConnection::isPointing(const QRectF &rect) const
{
    //check individual line segments
    for (int i = 1; i < _impl->points.size(); i++)
    {
        const QLineF line(_impl->points[i-1], _impl->points[i]);
        QLineF norm = line.normalVector(); norm.setLength(GraphConnectionSelectPad);
        if (QRectF(line.p2(), norm.p2()).intersects(rect)) return true;
    }

    //check arrow head
    return not _impl->arrowHead.intersected(rect).isEmpty();
}

QRectF GraphConnection::getBoundingRect(void) const
{
    QVector<QPointF> points = _impl->points;
    const auto arrowRect = _impl->arrowHead.boundingRect();
    points.push_back(arrowRect.topLeft());
    points.push_back(arrowRect.topRight());
    points.push_back(arrowRect.bottomRight());
    points.push_back(arrowRect.bottomLeft());
    return QPolygonF(points).boundingRect();
}

static int getAngle(const QPointF &p0_, const QPointF &p1_)
{
    //invert y for the vertical flip to fix angle
    const QPointF p0(p0_.x(), -p0_.y());
    const QPointF p1(p1_.x(), -p1_.y());
    return QLineF(p0, p1).angle();
}

static int deltaAcuteAngle(const int a0, const int a1)
{
    const int a = ((a0%360) - (a1%360) + 360)%360;
    return (a > 180)? (a - 180) : a;
}

static void makeLines(QVector<QPointF> &points, const QPointF p0, const int a0, const QPointF p1, const int a1)
{
    //this case uses three line connections
    if (deltaAcuteAngle(a0, a1) == 180)
    {
        //determine possible midpoints
        QPointF m0h((p0.x()+p1.x())/2, p0.y());
        QPointF m1h((p0.x()+p1.x())/2, p1.y());
        const auto a0mhdelta = deltaAcuteAngle(getAngle(p0, m0h), a0);
        const auto a1mhdelta = deltaAcuteAngle(getAngle(p1, m1h), a1);

        QPointF m0v(p0.x(), (p0.y()+p1.y())/2);
        QPointF m1v(p1.x(), (p0.y()+p1.y())/2);
        const auto a0mvdelta = deltaAcuteAngle(getAngle(p0, m0v), a0);
        const auto a1mvdelta = deltaAcuteAngle(getAngle(p1, m1v), a1);

        //pick the best midpoints
        QPointF m0, m1;
        if (a0mhdelta == 180 or a1mhdelta == 180) {m0 = m0v; m1 = m1v;}
        else if (a0mvdelta == 180 or a1mvdelta == 180) {m0 = m0h; m1 = m1h;}
        else if (a0mhdelta + a1mhdelta < a0mvdelta + a1mvdelta) {m0 = m0h; m1 = m1h;}
        else {m0 = m0v; m1 = m1v;}

        points.push_back(m0);
        points.push_back(m1);
    }

    //in this case, the connection is composed of two lines
    else
    {
        //determine possible midpoints
        QPointF midPoint0(p0.x(), p1.y());
        const auto a0m0delta = deltaAcuteAngle(getAngle(p0, midPoint0), a0);
        const auto a1m0delta = deltaAcuteAngle(getAngle(p1, midPoint0), a1);

        QPointF midPoint1(p1.x(), p0.y());
        const auto a0m1delta = deltaAcuteAngle(getAngle(p0, midPoint1), a0);
        const auto a1m1delta = deltaAcuteAngle(getAngle(p1, midPoint1), a1);

        //pick the best midpoint
        QPointF midPoint;
        if (a0m0delta == 180 or a1m0delta == 180) midPoint = midPoint1;
        else if (a0m1delta == 180 or a1m1delta == 180) midPoint = midPoint0;
        else if (a0m0delta + a1m0delta < a0m1delta + a1m1delta) midPoint = midPoint0;
        else midPoint = midPoint1;

        points.push_back(midPoint);
    }
}

static QLineF lineShorten(const QLineF &l)
{
    //translate, shorten, translate-back
    QLineF l0 = l;
    l0.setAngle(0);
    const qreal delta = std::min(GraphConnectionMaxCurve, l0.length()/2);
    l0.setP2(l0.p2() - QPointF(delta, 0));
    l0.setAngle(l.angle());
    return l0;
}

void GraphConnection::render(QPainter &painter)
{
    assert(_impl);
    //dont draw connections with missing endpoints
    if (not _impl->outputEp.isValid()) return;
    if (not _impl->inputEp.isValid()) return;

    //query the connectable info
    const auto outputAttrs = _impl->outputEp.getConnectableAttrs();
    const auto inputAttrs = _impl->inputEp.getConnectableAttrs();

    //make the minimal output protrusion
    const auto op0 = outputAttrs.point;
    QTransform otrans; otrans.rotate(outputAttrs.rotation);
    const auto op1 = outputAttrs.point + otrans.map(QPointF(GraphConnectionMinPling, 0));

    //make the minimal input protrusion
    const auto ip0 = inputAttrs.point;
    QTransform itrans; itrans.rotate(inputAttrs.rotation + 180);
    const auto ip1 = inputAttrs.point + itrans.map(QPointF(GraphConnectionMinPling+GraphConnectionArrowLen, 0));

    //create a path for the connection lines
    QVector<QPointF> points;
    points.push_back(op0);
    points.push_back(op1);
    makeLines(points, op1, outputAttrs.rotation, ip1, inputAttrs.rotation + 180);
    points.push_back(ip1);
    points.push_back(ip0);

    //create a painter path with curves for corners
    QPainterPath path(points.front());
    for (int i = 1; i < points.size()-1; i++)
    {
        const auto last = points[i-1];
        const auto curr = points[i];
        const auto next = points[i+1];
        path.lineTo(lineShorten(QLineF(last, curr)).p2());
        path.quadTo(curr, lineShorten(QLineF(next, curr)).p2());
    }
    path.lineTo(points.back());

    //draw the painter path
    QColor color(getSelected()?GraphConnectionHighlightColor:GraphConnectionDefaultColor);
    painter.setBrush(Qt::NoBrush);
    QPen pen(color);
    pen.setWidthF(GraphConnectionGirth);
    painter.setPen(pen);
    painter.drawPath(path);
    _impl->points = points;

    //create arrow head
    QTransform trans0; trans0.rotate(inputAttrs.rotation + GraphConnectionArrowAngle);
    QTransform trans1; trans1.rotate(inputAttrs.rotation - GraphConnectionArrowAngle);
    const auto p0 = trans0.map(QPointF(-GraphConnectionArrowLen, 0));
    const auto p1 = trans1.map(QPointF(-GraphConnectionArrowLen, 0));
    QPolygonF arrowHead;
    arrowHead << ip0 << (ip0+p0) << (ip0+p1);
    painter.setBrush(QBrush(color));
    painter.drawPolygon(arrowHead);
    _impl->arrowHead = arrowHead;
}

Poco::JSON::Object::Ptr GraphConnection::serialize(void) const
{
    auto obj = GraphObject::serialize();
    assert(this->getOutputEndpoint().isValid());
    assert(this->getInputEndpoint().isValid());
    obj->set("what", std::string("Connection"));
    obj->set("outputId", this->getOutputEndpoint().getObj()->getId().toStdString());
    obj->set("inputId", this->getInputEndpoint().getObj()->getId().toStdString());
    obj->set("outputKey", this->getOutputEndpoint().getKey().id.toStdString());
    obj->set("inputKey", this->getInputEndpoint().getKey().id.toStdString());
    return obj;
}

void GraphConnection::deserialize(Poco::JSON::Object::Ptr obj)
{
    auto outputId = QString::fromStdString(obj->getValue<std::string>("outputId"));
    auto inputId = QString::fromStdString(obj->getValue<std::string>("inputId"));
    auto outputKey = QString::fromStdString(obj->getValue<std::string>("outputKey"));
    auto inputKey = QString::fromStdString(obj->getValue<std::string>("inputKey"));

    auto draw = dynamic_cast<GraphDraw *>(this->parent());
    assert(draw != nullptr);

    //resolve IO objects by id
    QPointer<GraphObject> inputObj = nullptr;
    QPointer<GraphObject> outputObj = nullptr;
    for (const auto obj : draw->getGraphObjects(GRAPH_BLOCK | GRAPH_BREAKER))
    {
        if (obj->getId() == inputId) inputObj = obj;
        if (obj->getId() == outputId) outputObj = obj;
    }

    if (inputObj.isNull()) throw Pothos::Exception("GraphConnection::deserialize()", "cant resolve object with ID: '"+inputId.toStdString()+"'");
    if (outputObj.isNull()) throw Pothos::Exception("GraphConnection::deserialize()", "cant resolve object with ID: '"+outputId.toStdString()+"'");

    this->setupEndpoint(GraphConnectionEndpoint(inputObj, GraphConnectableKey(inputKey, GRAPH_CONN_INPUT)));
    this->setupEndpoint(GraphConnectionEndpoint(outputObj, GraphConnectableKey(outputKey, GRAPH_CONN_OUTPUT)));

    assert(this->getInputEndpoint().isValid());
    assert(this->getOutputEndpoint().isValid());

    GraphObject::deserialize(obj);
}
