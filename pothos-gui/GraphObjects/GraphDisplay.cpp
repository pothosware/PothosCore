// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphDisplay.hpp"
#include "GraphObjects/GraphBlock.hpp"
#include "GraphEditor/Constants.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <Pothos/Exception.hpp>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QStaticText>
#include <vector>
#include <iostream>
#include <cassert>

struct GraphDisplay::Impl
{
    Impl(void):
        changed(true)
    {
        return;
    }
    bool changed;

    QPointer<GraphBlock> block;

    QRectF mainRect;
};

GraphDisplay::GraphDisplay(QObject *parent):
    GraphObject(parent),
    _impl(new Impl())
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

    _impl->mainRect = QRectF(QPointF(0, 0), QSizeF(150, 100)); //TODO tmp
}

GraphBlock *GraphDisplay::getGraphBlock(void) const
{
    return _impl->block;
}

void GraphDisplay::handleBlockDestroyed(QObject *)
{
    //an endpoint was destroyed, schedule for deletion
    //however, the top level code should handle this deletion
    this->flagForDelete();
}

bool GraphDisplay::isPointing(const QRectF &rect) const
{
    return _impl->mainRect.intersects(rect);
}

QRectF GraphDisplay::getBoundingRect(void) const
{
    return _impl->mainRect;
}

void GraphDisplay::render(QPainter &painter)
{
    assert(_impl);

    //render text
    if (_impl->changed)
    {
        _impl->changed = false;
    }

    //QTransform trans;
    //trans.translate(this->getPosition().x(), this->getPosition().y());
    painter.translate(this->getPosition());

    auto pen = QPen(QColor(GraphObjectDefaultPenColor));
    pen.setWidthF(GraphObjectBorderWidth);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(GraphObjectDefaultFillColor)));

    painter.drawRect(_impl->mainRect);
}

Poco::JSON::Object::Ptr GraphDisplay::serialize(void) const
{
    auto obj = GraphObject::serialize();
    obj->set("what", std::string("Display"));
    obj->set("blockId", _impl->block->getId().toStdString());

    //TODO size

    return obj;
}

void GraphDisplay::deserialize(Poco::JSON::Object::Ptr obj)
{
    auto draw = dynamic_cast<GraphDraw *>(this->parent());
    assert(draw != nullptr);
    auto editor = draw->getGraphEditor();

    //locate the associated block
    auto blockId = QString::fromStdString(obj->getValue<std::string>("blockId"));
    auto graphObj = editor->getObjectById(blockId, GRAPH_BLOCK);
    if (graphObj == nullptr) throw Pothos::Exception("GraphDisplay::deserialize()", "cant resolve block with ID: '"+blockId.toStdString()+"'");
    auto graphBlock = dynamic_cast<GraphBlock *>(graphObj);
    assert(graphBlock != nullptr);
    this->setGraphBlock(graphBlock);

    //TODO size

    GraphObject::deserialize(obj);
}
