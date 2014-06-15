// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QObject>
#include <QString>
#include <QPointF>
#include <memory>
#include <vector>

class GraphBlockPort
{
public:
    GraphBlockPort(const QString &key = "", const QString &name = "")
    {
        _key = key;
        _name = name;
    }

    const QString &getKey(void) const
    {
        return _key;
    }

    const QString &getName(void) const
    {
        return _name;
    }

private:
    QString _key;
    QString _name;
};

class GraphBlockProp
{
public:
    GraphBlockProp(const QString &key = "", const QString &name = "")
    {
        _key = key;
        _name = name;
    }

    const QString &getKey(void) const
    {
        return _key;
    }

    const QString &getName(void) const
    {
        return _name;
    }

private:
    QString _key;
    QString _name;
};

class GraphBlock : public GraphObject
{
public:
    GraphBlock(QObject *parent);

    void setBlockDesc(const Poco::JSON::Object::Ptr &);
    const Poco::JSON::Object::Ptr &getBlockDesc(void) const;
    std::string getBlockDescPath(void) const;

    void setTitle(const QString &title);
    QString getTitle(void) const;

    bool isPointing(const QRectF &rect) const;

    QRectF getBoundingRect(void) const;

    void render(QPainter &painter);

    void addProperty(const GraphBlockProp &prop);
    const std::vector<GraphBlockProp> &getProperties(void) const;

    //! Get the param desc from the block description
    Poco::JSON::Object::Ptr getParamDesc(const QString &key) const;

    QString getPropertyValue(const QString &key) const;
    void setPropertyValue(const QString &key, const QString &value);

    bool getPropertyPreview(const QString &key) const;
    void setPropertyPreview(const QString &key, const bool value);

    void addInputPort(const GraphBlockPort &port);
    const std::vector<GraphBlockPort> &getInputPorts(void) const;

    void addOutputPort(const GraphBlockPort &port);
    const std::vector<GraphBlockPort> &getOutputPorts(void) const;

    void addSlotPort(const GraphBlockPort &port);
    const std::vector<GraphBlockPort> &getSlotPorts(void) const;

    void addSignalPort(const GraphBlockPort &port);
    const std::vector<GraphBlockPort> &getSignalPorts(void) const;

    std::vector<GraphConnectableKey> getConnectableKeys(void) const;
    GraphConnectableKey isPointingToConnectable(const QPointF &pos) const;
    GraphConnectableAttrs getConnectableAttrs(const GraphConnectableKey &key) const;

    Poco::JSON::Object::Ptr serialize(void) const;

    virtual void deserialize(Poco::JSON::Object::Ptr obj);

private:
    void update(void);
    void initPropertiesFromDesc(void);
    void renderStaticText(void);
    struct Impl;
    std::shared_ptr<Impl> _impl;
    QByteArray _blockDesc;
    std::vector<GraphBlockProp> _properties;
    std::vector<GraphBlockPort> _inputPorts;
    std::vector<GraphBlockPort> _outputPorts;
    std::vector<GraphBlockPort> _slotPorts;
    std::vector<GraphBlockPort> _signalPorts;
};
