// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QObject>
#include <QString>
#include <QPointF>
#include <memory>

/*!
 * A graph breaker implies a join among all breakers of the same mode name.
 * Breakers can be used to avoid clutter within a graph editor page,
 * and to make logical connections across the page boundary.
 */
class GraphBreaker : public GraphObject
{
    Q_OBJECT
public:
    GraphBreaker(QObject *parent);

    //! Set the direction mode of this breaker
    void setInput(const bool isInput);

    //! Get the direction mode of this breaker
    bool isInput(void) const;

    //! Set the name of the node that joins this breaker to others
    void setNodeName(const QString &name);

    //! Get the name of the node that joins this breaker to others
    const QString &getNodeName(void) const;

    bool isPointing(const QRectF &rect) const;

    QRectF boundingRect(void) const;

    void render(QPainter &painter);

    std::vector<GraphConnectableKey> getConnectableKeys(void) const;
    GraphConnectableKey isPointingToConnectable(const QPointF &pos) const;
    GraphConnectableAttrs getConnectableAttrs(const GraphConnectableKey &key) const;

    Poco::JSON::Object::Ptr serialize(void) const;

    virtual void deserialize(Poco::JSON::Object::Ptr obj);

private:
    void renderStaticText(void);
    struct Impl;
    std::shared_ptr<Impl> _impl;
};
