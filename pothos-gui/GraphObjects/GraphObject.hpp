// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphEndpoint.hpp"
#include <QList>
#include <QGraphicsObject>
#include <QPointF>
#include <QRectF>
#include <vector>
#include <memory>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>

class QObject;
class QPainter;
class GraphObject;
class GraphDraw;

//! Represent a list of graph objects
typedef QList<GraphObject *> GraphObjectList;

//! Base class for graph objects
class GraphObject : public QGraphicsObject
{
    Q_OBJECT
public:
    GraphObject(QObject *parent);

    ~GraphObject(void);

    GraphDraw *draw(void) const;

    virtual QRectF boundingRect(void) const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    virtual QPainterPath shape(void) const;

    virtual void setId(const QString &id);
    const QString &getId(void) const;

    //! render without a painter to do-precalculations
    void prerender(void);
    virtual void render(QPainter &painter);

    void rotateLeft(void);
    void rotateRight(void);

    //! empty string when not pointing, otherwise connectable key
    virtual std::vector<GraphConnectableKey> getConnectableKeys(void) const;
    virtual GraphConnectableKey isPointingToConnectable(const QPointF &pos) const;
    virtual GraphConnectableAttrs getConnectableAttrs(const GraphConnectableKey &key) const;
    virtual void renderConnectablePoints(QPainter &painter);

    bool isFlaggedForDelete(void) const;
    void flagForDelete(void);

    virtual Poco::JSON::Object::Ptr serialize(void) const;

    virtual void deserialize(Poco::JSON::Object::Ptr obj);

signals:
    void IDChanged(const QString &);

protected:
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};
