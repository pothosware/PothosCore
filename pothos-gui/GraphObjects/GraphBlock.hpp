// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QStringList>
#include <QObject>
#include <QString>
#include <QPointF>
#include <memory>
#include <vector>

class GraphBlock : public GraphObject
{
public:
    GraphBlock(QObject *parent);

    void setBlockDesc(const Poco::JSON::Object::Ptr &);
    const Poco::JSON::Object::Ptr &getBlockDesc(void) const;
    std::string getBlockDescPath(void) const;

    void setPortDesc(const Poco::JSON::Array::Ptr &, const Poco::JSON::Array::Ptr &);

    //! Does this graph block represent a display widget
    bool isDisplayWidget(void) const;

    void setTitle(const QString &title);
    QString getTitle(void) const;

    bool isPointing(const QRectF &rect) const;

    QRectF getBoundingRect(void) const;

    //! indicate an external change -- thats not applied through the other setters
    void changed(void);

    void render(QPainter &painter);

    //! Set an error message when trying to eval the block
    void clearBlockErrorMsgs(void);
    void addBlockErrorMsg(const QString &msg);
    const QStringList &getBlockErrorMsgs(void) const;

    void addProperty(const QString &key);
    const QStringList &getProperties(void) const;

    //! Get the param desc from the block description
    Poco::JSON::Object::Ptr getParamDesc(const QString &key) const;

    QString getPropertyValue(const QString &key) const;
    void setPropertyValue(const QString &key, const QString &value);

    QString getPropertyName(const QString &key) const;
    void setPropertyName(const QString &key, const QString &name);

    //! Get the property display text: varies from actual value, enum name, error...
    QString getPropertyDisplayText(const QString &key) const;

    //! Set the error message when trying to eval this property -- blank msg for no error
    void setPropertyErrorMsg(const QString &key, const QString &msg);
    const QString &getPropertyErrorMsg(const QString &key) const;

    //! Set a descriptive type string for this property
    void setPropertyTypeStr(const QString &key, const std::string &type);
    const std::string &getPropertyTypeStr(const QString &key) const;

    bool getPropertyPreview(const QString &key) const;
    void setPropertyPreview(const QString &key, const bool value);

    void addInputPort(const QString &portKey);
    const QStringList &getInputPorts(void) const;

    void addOutputPort(const QString &portKey);
    const QStringList &getOutputPorts(void) const;

    void addSlotPort(const QString &portKey);
    const QStringList &getSlotPorts(void) const;

    void addSignalPort(const QString &portKey);
    const QStringList &getSignalPorts(void) const;

    //! Set a descriptive type string for input ports
    void setInputPortTypeStr(const QString &key, const std::string &type);
    const std::string &getInputPortTypeStr(const QString &key) const;

    //! Set a descriptive type string for output ports
    void setOutputPortTypeStr(const QString &key, const std::string &type);
    const std::string &getOutputPortTypeStr(const QString &key) const;

    std::vector<GraphConnectableKey> getConnectableKeys(void) const;
    GraphConnectableKey isPointingToConnectable(const QPointF &pos) const;
    GraphConnectableAttrs getConnectableAttrs(const GraphConnectableKey &key) const;

    Poco::JSON::Object::Ptr serialize(void) const;

    virtual void deserialize(Poco::JSON::Object::Ptr obj);

    //! affinity zone support
    const QString &getAffinityZone(void) const;
    void setAffinityZone(const QString &zone);

    void update(void);
private:
    void initPropertiesFromDesc(void);
    void initInputsFromDesc(void);
    void initOutputsFromDesc(void);
    void renderStaticText(void);
    struct Impl;
    std::shared_ptr<Impl> _impl;
    QStringList _properties;
    QStringList _inputPorts;
    QStringList _outputPorts;
    QStringList _slotPorts;
    QStringList _signalPorts;
};
