// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <Pothos/Proxy.hpp>
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

    //! Set an error message when trying to eval the block -- blank msg for no error
    void setBlockErrorMsg(const QString &msg);
    const QString &getBlockErrorMsg(void) const;

    void addProperty(const GraphBlockProp &prop);
    const std::vector<GraphBlockProp> &getProperties(void) const;

    //! Get the param desc from the block description
    Poco::JSON::Object::Ptr getParamDesc(const QString &key) const;

    QString getPropertyValue(const QString &key) const;
    void setPropertyValue(const QString &key, const QString &value);

    //! Set the error message when trying to eval this property -- blank msg for no error
    void setPropertyErrorMsg(const QString &key, const QString &msg);
    const QString &getPropertyErrorMsg(const QString &key) const;

    //! Set a descriptive type string for this property
    void setPropertyTypeStr(const QString &key, const std::string &type);
    const std::string &getPropertyTypeStr(const QString &key) const;

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

    Pothos::Proxy getBlockEval(void) const
    {
        return _blockEval;
    }

    //! affinity zone support
    const QString &getAffinityZone(void) const;
    void setAffinityZone(const QString &zone);

    void update(void);
private:
    void initPropertiesFromDesc(void);
    void renderStaticText(void);
    struct Impl;
    std::shared_ptr<Impl> _impl;
    std::vector<GraphBlockProp> _properties;
    std::vector<GraphBlockPort> _inputPorts;
    std::vector<GraphBlockPort> _outputPorts;
    std::vector<GraphBlockPort> _slotPorts;
    std::vector<GraphBlockPort> _signalPorts;
    Pothos::Proxy _blockEval;
};
