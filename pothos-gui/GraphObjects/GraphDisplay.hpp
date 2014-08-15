// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QObject>
#include <QString>
#include <QPointF>
#include <memory>

class GraphBlock;

/*!
 * A graph display represents a widget from a GraphBlock in displayWidget mode.
 */
class GraphDisplay : public GraphObject
{
    Q_OBJECT
public:
    GraphDisplay(QObject *parent);
    ~GraphDisplay(void);

    void setGraphBlock(GraphBlock *block);
    GraphBlock *getGraphBlock(void) const;

    bool isPointing(const QRectF &rect) const;

    QPainterPath shape(void) const;

    void render(QPainter &painter);

    Poco::JSON::Object::Ptr serialize(void) const;

    virtual void deserialize(Poco::JSON::Object::Ptr obj);

private slots:
    void handleBlockDestroyed(QObject *);

protected:
    bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};
