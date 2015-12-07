// Copyright (c) 2013-2015 Josh Blum
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
 * A graph display represents a widget from a GraphBlock in graphWidget mode.
 */
class GraphWidget : public GraphObject
{
    Q_OBJECT
public:
    GraphWidget(QObject *parent);
    ~GraphWidget(void);

    void setGraphBlock(GraphBlock *block);
    GraphBlock *getGraphBlock(void) const;

    //! True when the container widget has focus
    bool containerHasFocus(void) const;

    QPainterPath shape(void) const;

    Poco::JSON::Object::Ptr serialize(void) const;

    virtual void deserialize(Poco::JSON::Object::Ptr obj);

private slots:
    void handleBlockDestroyed(QObject *);
    void handleWidgetResized(void);
    void handleBlockIdChanged(const QString &id);
    void handleBlockEvalDone(void);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};
