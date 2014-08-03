// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphBreaker.hpp"
#include "GraphEditor/Constants.hpp"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QStaticText>
#include <vector>
#include <iostream>
#include <cassert>

struct GraphBreaker::Impl
{
    Impl(void):
        isInput(true),
        changed(true)
    {
        return;
    }
    bool isInput;
    bool changed;
    QString nodeName;
    QStaticText titleText;
    QPolygonF polygon;
    QPointF connectPoint;
};

GraphBreaker::GraphBreaker(QObject *parent):
    GraphObject(parent),
    _impl(new Impl())
{
    return;
}

void GraphBreaker::setInput(const bool isInput)
{
    assert(_impl);
    _impl->isInput = isInput;
}

bool GraphBreaker::isInput(void) const
{
    assert(_impl);
    return _impl->isInput;
}

void GraphBreaker::setNodeName(const QString &name)
{
    assert(_impl);
    _impl->nodeName = name;
    _impl->changed = true;
}

const QString &GraphBreaker::getNodeName(void) const
{
    assert(_impl);
    return _impl->nodeName;
}

bool GraphBreaker::isPointing(const QRectF &rect) const
{
    return not _impl->polygon.intersected(rect).isEmpty();
}

QRectF GraphBreaker::getBoundingRect(void) const
{
    return _impl->polygon.boundingRect();
}

std::vector<GraphConnectableKey> GraphBreaker::getConnectableKeys(void) const
{
    std::vector<GraphConnectableKey> keys;
    keys.push_back(GraphConnectableKey("0", this->isInput()?GRAPH_CONN_INPUT:GRAPH_CONN_OUTPUT));
    return keys;
}

GraphConnectableKey GraphBreaker::isPointingToConnectable(const QPointF &pos) const
{
    assert(_impl);
    GraphConnectableKey key("", this->isInput()?GRAPH_CONN_INPUT:GRAPH_CONN_OUTPUT);
    if (_impl->polygon.containsPoint(pos, Qt::OddEvenFill)) key.id = "0";
    return key;
}

GraphConnectableAttrs GraphBreaker::getConnectableAttrs(const GraphConnectableKey &) const
{
    assert(_impl);
    GraphConnectableAttrs attrs;
    attrs.rotation = this->getRotation();
    if (this->isInput()) attrs.rotation += 180;
    attrs.direction = this->isInput()?GRAPH_CONN_INPUT:GRAPH_CONN_OUTPUT;
    attrs.point = _impl->connectPoint;
    return attrs;
}

void GraphBreaker::renderStaticText(void)
{
    assert(_impl);
    _impl->titleText = QStaticText(QString("<span style='font-size:%1;'><b>%2</b></span>")
        .arg(GraphBreakerTitleFontSize)
        .arg(_impl->nodeName.toHtmlEscaped()));
}

void GraphBreaker::render(QPainter &painter)
{
    assert(_impl);
    //render text
    if (_impl->changed)
    {
        _impl->changed = false;
        this->renderStaticText();
    }

    //setup rotations and translations
    QTransform trans;
    trans.translate(this->getPosition().x(), this->getPosition().y());
    painter.translate(this->getPosition());

    //dont rotate past 180 because we just do a breaker flip
    //this way text only ever has 2 rotations
    trans.rotate(this->getRotation() % 180);
    painter.rotate(this->getRotation() % 180);
    const bool breakerFlip = this->getRotation() >= 180;

    //set painter for drawing the figure
    auto pen = QPen(QColor(GraphObjectDefaultPenColor));
    pen.setWidthF(GraphObjectBorderWidth);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(GraphObjectDefaultFillColor)));

    qreal w = _impl->titleText.size().width() + 2*GraphBreakerTitleHPad;
    qreal h = _impl->titleText.size().height() + 2*GraphBreakerTitleVPad;

    QPolygonF polygon;
    const bool flipStyle = (this->isInput() and not breakerFlip) or (not this->isInput() and breakerFlip);
    if (flipStyle)
    {
        polygon << QPointF(0, 0);
        for (int i = 0; i <= 6; i++)
        {
            const qreal jut = ((i % 2) == (this->isInput()?0:1))? 0 : GraphBreakerEdgeJut;
            polygon << QPointF(w+jut, h*(i/6.));
        }
        polygon << QPointF(0, h);
    }
    else
    {
        polygon << QPointF(0, 0);
        polygon << QPointF(w, 0);
        polygon << QPointF(w, h);
        for (int i = 6; i >= 0; i--)
        {
            const qreal jut = ((i % 2) == (this->isInput()?0:1))? 0 : GraphBreakerEdgeJut;
            polygon << QPointF(-jut, h*(i/6.));
        }
    }

    QPointF p(-w/2, -h/2);
    polygon.translate(p);
    painter.save();
    if (getSelected()) painter.setPen(QColor(GraphObjectHighlightPenColor));
    painter.drawPolygon(polygon);
    painter.restore();
    _impl->polygon = trans.map(polygon);

    const auto textOff = QPointF(GraphBreakerTitleHPad, GraphBreakerTitleVPad);
    painter.drawStaticText(p+textOff, _impl->titleText);

    //connection point
    const auto connectionPoint = QPointF(flipStyle?-GraphObjectBorderWidth:w+GraphObjectBorderWidth, h/2) + p;
    _impl->connectPoint = trans.map(connectionPoint);
}

Poco::JSON::Object::Ptr GraphBreaker::serialize(void) const
{
    auto obj = GraphObject::serialize();
    obj->set("what", std::string("Breaker"));
    obj->set("nodeName", this->getNodeName().toStdString());
    obj->set("isInput", this->isInput());
    return obj;
}

void GraphBreaker::deserialize(Poco::JSON::Object::Ptr obj)
{
    auto nodeName = QString::fromStdString(obj->getValue<std::string>("nodeName"));
    auto isInput = obj->getValue<bool>("isInput");

    this->setInput(isInput);
    this->setNodeName(nodeName);

    GraphObject::deserialize(obj);
}
