// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphConnection.hpp"
#include "GraphObjects/GraphBreaker.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/Constants.hpp"
#include <Pothos/Exception.hpp>
#include <Poco/Logger.h>
#include <QPainter>
#include <QPolygonF>
#include <QStaticText>
#include <iostream>
#include <algorithm> //std::find
#include <cassert>
#include <QtMath> //qCos, radians

static std::string directionToStr(const GraphConnectableDirection direction)
{
    switch (direction)
    {
    case GRAPH_CONN_INPUT: return "input";
    case GRAPH_CONN_OUTPUT: return "output";
    case GRAPH_CONN_SLOT: return "slot";
    case GRAPH_CONN_SIGNAL: return "signal";
    }
    return "";
}

struct GraphConnection::Impl
{
    Impl(void)
    {
        return;
    }

    GraphConnectionEndpoint inputEp;
    GraphConnectionEndpoint outputEp;

    std::vector<SigSlotPair> sigSlotsEndpointPairs;

    QStaticText lineText;

    QVector<QPointF> points;
    QPolygonF arrowHead;
    QRectF textRect;
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
    switch (ep.getKey().direction)
    {
    case GRAPH_CONN_INPUT:
    case GRAPH_CONN_SLOT: _impl->inputEp = ep; break;
    case GRAPH_CONN_OUTPUT:
    case GRAPH_CONN_SIGNAL: _impl->outputEp = ep; break;
    }

    //connected deleted signal so the connection deletes with the endpoint's parent object
    connect(ep.getObj(), SIGNAL(destroyed(QObject *)), this, SLOT(handleEndPointDestroyed(QObject *)));

    //connect eval signal to check if the endpoint exists and delete this connection
    if (dynamic_cast<GraphBlock *>(ep.getObj().data()) != nullptr)
    {
        connect(ep.getObj(), SIGNAL(evalDoneEvent(void)), this, SLOT(handleEndPointEventRecheck(void)));
    }

    this->markChanged();
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

bool GraphConnection::isSignalOrSlot(void) const
{
    return
        this->getOutputEndpoint().getKey().direction == GRAPH_CONN_SLOT or
        this->getOutputEndpoint().getKey().direction == GRAPH_CONN_SIGNAL or
        this->getInputEndpoint().getKey().direction == GRAPH_CONN_SLOT or
        this->getInputEndpoint().getKey().direction == GRAPH_CONN_SIGNAL;
}

const std::vector<SigSlotPair> &GraphConnection::getSigSlotPairs(void) const
{
    return _impl->sigSlotsEndpointPairs;
}

void GraphConnection::setSigSlotPairs(const std::vector<SigSlotPair> &pairs)
{
    _impl->sigSlotsEndpointPairs = pairs;
    this->markChanged();
}

static void doSigSlotWarning(const QString &name, const GraphConnectionEndpoint &ep)
{
    poco_warning_f3(Poco::Logger::get("PothosGui.GraphConnection.addSigSlotPair"),
        "cant find %s '%s' in %s", directionToStr(ep.getConnectableAttrs().direction), name.toStdString(), ep.getObj()->getId().toStdString());
}

void GraphConnection::addSigSlotPair(const SigSlotPair &sigSlot)
{
    //check that the output endpoint is possible
    auto epOut = this->getOutputEndpoint();
    auto blockOut = dynamic_cast<GraphBlock *>(epOut.getObj().data());
    if (blockOut != nullptr and epOut.getConnectableAttrs().direction == GRAPH_CONN_SIGNAL)
    {
        const auto &keys = blockOut->getSignalPorts();
        if (std::find(keys.begin(), keys.end(), sigSlot.first) == keys.end()) return doSigSlotWarning(sigSlot.first, epOut);
    }
    else if (sigSlot.first != epOut.getKey().id) return doSigSlotWarning(sigSlot.first, epOut);

    //check that the input endpoint is possible
    auto epIn = this->getInputEndpoint();
    auto blockIn = dynamic_cast<GraphBlock *>(epIn.getObj().data());
    if (blockIn != nullptr and epIn.getConnectableAttrs().direction == GRAPH_CONN_SLOT)
    {
        const auto &keys = blockIn->getSlotPorts();
        if (std::find(keys.begin(), keys.end(), sigSlot.second) == keys.end()) return doSigSlotWarning(sigSlot.second, epIn);
    }
    else if (sigSlot.second != epIn.getKey().id) return doSigSlotWarning(sigSlot.second, epIn);

    //optional remove, then add to the end of the list
    this->removeSigSlotPair(sigSlot);
    _impl->sigSlotsEndpointPairs.push_back(sigSlot);
    this->markChanged();
}

void GraphConnection::removeSigSlotPair(const SigSlotPair &sigSlot)
{
    auto &v = _impl->sigSlotsEndpointPairs;
    auto it = std::find(v.begin(), v.end(), sigSlot);
    if (it != v.end()) v.erase(it);
    this->markChanged();
}

std::vector<std::pair<GraphConnectionEndpoint, GraphConnectionEndpoint>> GraphConnection::getEndpointPairs(void) const
{
    std::vector<std::pair<GraphConnectionEndpoint, GraphConnectionEndpoint>> pairs;
    if (not this->isSignalOrSlot())
    {
        pairs.push_back(std::make_pair(this->getOutputEndpoint(), this->getInputEndpoint()));
    }
    else for (const auto &pair : this->getSigSlotPairs())
    {
        pairs.push_back(std::make_pair(
            GraphConnectionEndpoint(this->getOutputEndpoint().getObj(), GraphConnectableKey(pair.first, this->getOutputEndpoint().getKey().direction)),
            GraphConnectionEndpoint(this->getInputEndpoint().getObj(), GraphConnectableKey(pair.second, this->getInputEndpoint().getKey().direction))));
    }
    return pairs;
}

QString GraphConnection::getKeyName(const QString &portKey, const GraphConnectionEndpoint &ep)
{
    switch(ep.getConnectableAttrs().direction)
    {
    case GRAPH_CONN_INPUT: return tr("Input %1").arg(portKey);
    case GRAPH_CONN_OUTPUT: return tr("Output %1").arg(portKey);
    default: break;
    }
    return portKey;
}

void GraphConnection::handleEndPointDestroyed(QObject *)
{
    //an endpoint was destroyed, schedule for deletion
    //however, the top level code should handle this deletion
    this->flagForDelete();
}

void GraphConnection::handleEndPointEventRecheck(void)
{
    bool foundOutput = false;
    for (const auto &key : this->getOutputEndpoint().getObj()->getConnectableKeys())
    {
        if (key == this->getOutputEndpoint().getKey()) foundOutput = true;
    }

    bool foundInput = false;
    for (const auto &key : this->getInputEndpoint().getObj()->getConnectableKeys())
    {
        if (key == this->getInputEndpoint().getKey()) foundInput = true;
    }

    //the endpoint is missing
    if (not foundOutput or not foundInput) this->flagForDelete();
}

QPainterPath GraphConnection::shape(void) const
{
    QPainterPath path;

    //individual line segments
    for (int i = 1; i < _impl->points.size(); i++)
    {
        const QLineF line(_impl->points[i-1], _impl->points[i]);
        QLineF norm = line.normalVector(); norm.setLength(GraphConnectionSelectPad);
        path.addRect(QRectF(line.p2(), norm.p2()));
    }

    //arrow head
    path.addPolygon(_impl->arrowHead);

    //text
    path.addRect(_impl->textRect);

    return path;
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
    assert(this->rotation() == 0.0);
    assert(this->pos() == QPointF());

    //dont draw connections with missing endpoints
    if (not _impl->outputEp.isValid()) return;
    if (not _impl->inputEp.isValid()) return;

    //re-render the static texts upon change
    if (this->isChanged())
    {
        this->clearChanged();
        QString text;
        for (const auto &pair : this->getSigSlotPairs())
        {
            if (not text.isEmpty()) text += "<br />";
            //abbreviate the text by only showing the destinations
            //text += QString("%1:%2")
            //    .arg(this->getKeyName(pair.first, this->getOutputEndpoint()).toHtmlEscaped())
            //    .arg(this->getKeyName(pair.second, this->getInputEndpoint()).toHtmlEscaped());
            text += this->getKeyName(pair.second, this->getInputEndpoint()).toHtmlEscaped();
        }
        if (text.isEmpty()) text = tr("<b>Empty</b>");
        _impl->lineText = QStaticText(QString("<span style='color:%1;font-size:%2;'>%3</span>")
            .arg(GraphConnectionLineTextColor)
            .arg(GraphConnectionLineTextFontSize)
            .arg(text));
        QTextOption to; to.setWrapMode(QTextOption::NoWrap);
        _impl->lineText.setTextOption(to);
    }

    //query the connectable info
    auto outputAttrs = _impl->outputEp.getConnectableAttrs();
    outputAttrs.point = this->mapFromItem(_impl->outputEp.getObj(), outputAttrs.point);
    auto inputAttrs = _impl->inputEp.getConnectableAttrs();
    inputAttrs.point = this->mapFromItem(_impl->inputEp.getObj(), inputAttrs.point);

    //make the minimal output protrusion
    const auto op0 = outputAttrs.point;
    QTransform otrans; otrans.rotate(outputAttrs.rotation);
    const auto op1 = outputAttrs.point + otrans.map(QPointF(GraphConnectionMinPling, 0));

    //make the minimal input protrusion
    QTransform itrans; itrans.rotate(inputAttrs.rotation);
    const auto ip0 = inputAttrs.point + itrans.map(QPointF(GraphConnectionArrowLen, 0));
    const auto ip1 = inputAttrs.point + itrans.map(QPointF(GraphConnectionMinPling+GraphConnectionArrowLen, 0));

    //create a path for the connection lines
    QVector<QPointF> points;
    points.push_back(op0);
    points.push_back(op1);
    makeLines(points, op1, outputAttrs.rotation, ip1, inputAttrs.rotation);
    points.push_back(ip1);
    points.push_back(ip0);

    //create a painter path with curves for corners
    QLineF largestLine;
    QPainterPath path(points.front());
    for (int i = 1; i < points.size()-1; i++)
    {
        const auto last = points[i-1];
        const auto curr = points[i];
        const auto next = points[i+1];
        const QLineF line(last, curr);
        if (line.length() > largestLine.length()) largestLine = line;
        path.lineTo(lineShorten(line).p2());
        path.quadTo(curr, lineShorten(QLineF(next, curr)).p2());
    }
    path.lineTo(points.back());

    //draw the painter path
    QColor color(GraphConnectionDefaultColor);
    if (this->isSelected()) color = GraphConnectionHighlightColor;
    else if (not this->isEnabled()) color = GraphConnectionDisabledColor;
    painter.setBrush(Qt::NoBrush);
    QPen pen(color);
    pen.setWidthF(GraphConnectionGirth);
    if (this->isSignalOrSlot()) pen.setStyle(Qt::DashLine);
    painter.setPen(pen);
    painter.drawPath(path);
    _impl->points = points;

    //draw an X for disabled
    if (not this->isEnabled())
    {
        painter.save();
        const qreal len(GraphConnectionDisabledXLen/2.0);
        QLineF line0(QPointF(+len, +len), QPointF(-len, -len));
        QLineF line1(QPointF(-len, +len), QPointF(+len, -len));
        painter.translate((largestLine.p1() + largestLine.p2())/2.0);
        painter.drawLine(line0);
        painter.drawLine(line1);
        painter.restore();
    }

    //draw text
    if (this->isSignalOrSlot())
    {
        painter.save();
        const auto &text = _impl->lineText;
        painter.translate((largestLine.p1() + largestLine.p2())/2.0);
        painter.rotate(int(largestLine.angle())%180);
        const auto hs = this->getSigSlotPairs().size()/std::ceil(this->getSigSlotPairs().size()/2.0);
        const QRectF textRect(QPointF(-text.size().width()/2, -text.size().height()/hs - GraphConnectionGirth), text.size());
        painter.drawStaticText(textRect.topLeft(), text);
        _impl->textRect = painter.worldTransform().mapRect(textRect);
        painter.restore();
    }

    //create arrow head
    QTransform trans0; trans0.rotate(inputAttrs.rotation + 180 + GraphConnectionArrowAngle);
    QTransform trans1; trans1.rotate(inputAttrs.rotation + 180 - GraphConnectionArrowAngle);
    const auto diagonalLength = GraphConnectionArrowLen/qCos(qDegreesToRadians(GraphConnectionArrowAngle));
    const auto p0 = trans0.map(QPointF(-diagonalLength, 0));
    const auto p1 = trans1.map(QPointF(-diagonalLength, 0));
    QPolygonF arrowHead;
    const auto tip = inputAttrs.point;
    arrowHead << tip << (tip+p0) << (tip+p1);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(color));
    painter.drawPolygon(arrowHead);
    _impl->arrowHead = arrowHead;
}

/***********************************************************************
 * serialize - connection to JSON
 **********************************************************************/
static void endpointSerialize(Poco::JSON::Object::Ptr obj, const GraphConnectionEndpoint &ep)
{
    const auto key = directionToStr(ep.getConnectableAttrs().direction);
    obj->set(key+"Id", ep.getObj()->getId().toStdString());
    obj->set(key+"Key", ep.getKey().id.toStdString());
}

Poco::JSON::Object::Ptr GraphConnection::serialize(void) const
{
    auto obj = GraphObject::serialize();
    assert(this->getOutputEndpoint().isValid());
    assert(this->getInputEndpoint().isValid());
    obj->set("what", std::string("Connection"));
    endpointSerialize(obj, this->getOutputEndpoint());
    endpointSerialize(obj, this->getInputEndpoint());

    //save signal-slots connection pairs
    Poco::JSON::Array::Ptr sigSlots(new Poco::JSON::Array());
    for (const auto &pair : this->getSigSlotPairs())
    {
        Poco::JSON::Array::Ptr sigSlot(new Poco::JSON::Array());
        sigSlot->add(pair.first.toStdString());
        sigSlot->add(pair.second.toStdString());
        sigSlots->add(sigSlot);
    }
    if (sigSlots->size() > 0) obj->set("sigSlots", sigSlots);

    return obj;
}

/***********************************************************************
 * deserialize - JSON to connection
 **********************************************************************/
static GraphConnectionEndpoint endpointDeserialize(GraphDraw *draw, Poco::JSON::Object::Ptr obj, const GraphConnectableDirection &direction)
{
    const auto key = directionToStr(direction);
    if (obj->has(key+"Id") and obj->has(key+"Key"))
    {
        auto portId = QString::fromStdString(obj->getValue<std::string>(key+"Id"));
        auto portKey = QString::fromStdString(obj->getValue<std::string>(key+"Key"));
        auto graphObj = draw->getObjectById(portId, ~GRAPH_CONNECTION);
        if (graphObj == nullptr) throw Pothos::NotFoundException("GraphConnection::deserialize()", "cant resolve object with ID: '"+portId.toStdString()+"'");
        return GraphConnectionEndpoint(graphObj, GraphConnectableKey(portKey, direction));
    }
    return GraphConnectionEndpoint();
}

void GraphConnection::deserialize(Poco::JSON::Object::Ptr obj)
{
    auto outputEp = endpointDeserialize(this->draw(), obj, GRAPH_CONN_OUTPUT);
    auto inputEp = endpointDeserialize(this->draw(), obj, GRAPH_CONN_INPUT);
    auto slotEp = endpointDeserialize(this->draw(), obj, GRAPH_CONN_SLOT);
    auto signalEp = endpointDeserialize(this->draw(), obj, GRAPH_CONN_SIGNAL);

    if (outputEp.isValid()) this->setupEndpoint(outputEp);
    if (inputEp.isValid()) this->setupEndpoint(inputEp);
    if (slotEp.isValid()) this->setupEndpoint(slotEp);
    if (signalEp.isValid()) this->setupEndpoint(signalEp);

    assert(this->getInputEndpoint().isValid());
    assert(this->getOutputEndpoint().isValid());

    //restore signal-slots connection pairs
    Poco::JSON::Array::Ptr sigSlots;
    if (obj->isArray("sigSlots")) sigSlots = obj->getArray("sigSlots");
    if (sigSlots) for (size_t i = 0; i < sigSlots->size(); i++)
    {
        if (not sigSlots->isArray(i)) continue;
        auto sigSlot = sigSlots->getArray(i);
        if (sigSlot->size() != 2) continue;
        auto pair = std::make_pair(
            QString::fromStdString(sigSlot->getElement<std::string>(0)),
            QString::fromStdString(sigSlot->getElement<std::string>(1)));
        this->addSigSlotPair(pair);
    }

    GraphObject::deserialize(obj);
}
