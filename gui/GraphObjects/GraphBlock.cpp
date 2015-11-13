// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "PothosGuiUtils.hpp" //get object map
#include "GraphObjects/GraphBlockImpl.hpp"
#include "GraphEditor/Constants.hpp"
#include "BlockTree/BlockCache.hpp"
#include "AffinitySupport/AffinityZonesDock.hpp"
#include "ColorUtils/ColorUtils.hpp"
#include <Pothos/Exception.hpp>
#include <QGraphicsScene>
#include <QAction>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QFontMetrics>
#include <iostream>
#include <cassert>
#include <algorithm> //min/max

GraphBlock::GraphBlock(QObject *parent):
    GraphObject(parent),
    _impl(new Impl())
{
    this->setFlag(QGraphicsItem::ItemIsMovable);
}

std::string GraphBlock::getBlockDescPath(void) const
{
    return _impl->blockDesc->getValue<std::string>("path");
}

const Poco::JSON::Object::Ptr &GraphBlock::getBlockDesc(void) const
{
    assert(_impl);
    return _impl->blockDesc;
}

bool GraphBlock::isGraphWidget(void) const
{
    return this->getBlockDesc()->optValue<std::string>("mode", "") == "graphWidget";
}

QWidget *GraphBlock::getGraphWidget(void) const
{
    return _impl->graphWidget.data();
}

void GraphBlock::setGraphWidget(QWidget *widget)
{
    if (_impl->graphWidget == widget) return;
    _impl->graphWidget = widget;
    this->markChanged();
}

void GraphBlock::setTitle(const QString &title)
{
    if (_impl->title == title) return;
    _impl->title = title;
    this->markChanged();
}

QString GraphBlock::getTitle(void) const
{
    return _impl->title;
}

void GraphBlock::clearBlockErrorMsgs(void)
{
    if (_impl->blockErrorMsgs.empty()) return;
    _impl->blockErrorMsgs.clear();
    this->markChanged();
}

void GraphBlock::addBlockErrorMsg(const QString &msg)
{
    assert(not msg.isEmpty());
    _impl->blockErrorMsgs.push_back(msg);
    this->markChanged();
}

const QStringList &GraphBlock::getBlockErrorMsgs(void) const
{
    return _impl->blockErrorMsgs;
}

void GraphBlock::addProperty(const QString &key)
{
    _properties.push_back(key);
    this->markChanged();
}

const QStringList &GraphBlock::getProperties(void) const
{
    return _properties;
}

Poco::JSON::Object::Ptr GraphBlock::getParamDesc(const QString &key) const
{
    Poco::JSON::Object::Ptr paramDesc;
    for (const auto &paramObj : *this->getBlockDesc()->getArray("params"))
    {
        const auto param = paramObj.extract<Poco::JSON::Object::Ptr>();
        if (param->getValue<std::string>("key") == key.toStdString()) paramDesc = param;
    }
    return paramDesc;
}

QString GraphBlock::getPropertyDisplayText(const QString &key) const
{
    auto value = this->getPropertyValue(key);
    auto paramDesc = this->getParamDesc(key);
    if (paramDesc and paramDesc->isArray("options"))
    {
        for (const auto &optionObj : *paramDesc->getArray("options"))
        {
            const auto option = optionObj.extract<Poco::JSON::Object::Ptr>();
            if (value == QString::fromStdString(option->getValue<std::string>("value")))
            {
                return QString::fromStdString(option->getValue<std::string>("name"));
            }
        }
    }

    //strip enclosing control characters to save space
    if (value.size() >= 2 and (
        (value.startsWith("\"") and value.endsWith("\"")) or
        (value.startsWith("'") and value.endsWith("'")) or
        (value.startsWith("(") and value.endsWith(")")) or
        (value.startsWith("[") and value.endsWith("]")) or
        (value.startsWith("{") and value.endsWith("}"))
    ))
    {
        value.remove(-1, 1).remove(0, 1);
    }

    return value;
}

QString GraphBlock::getPropertyValue(const QString &key) const
{
    auto it = _impl->propertiesValues.find(key);
    if (it != _impl->propertiesValues.end()) return it->second;
    return "";
}

void GraphBlock::setPropertyValue(const QString &key, const QString &value)
{
    if (_impl->propertiesValues[key] == value) return;
    _impl->propertiesValues[key] = value;
    this->markChanged();
}

QString GraphBlock::getPropertyName(const QString &key) const
{
    auto it = _impl->propertiesNames.find(key);
    if (it != _impl->propertiesNames.end()) return it->second;
    return key;
}

void GraphBlock::setPropertyName(const QString &key, const QString &name)
{
    if (_impl->propertiesNames[key] == name) return;
    _impl->propertiesNames[key] = name;
    this->markChanged();
}

/*!
 * determine if a value is valid (boolean true)
 * empty data types are considered to be false.
 */
static bool isValid(const QString &value)
{
    if (value.isEmpty()) return false;
    if (value == "\"\"") return false;
    if (value == "''") return false;
    if (value == "0") return false;
    if (value == "0.0") return false;
    if (value == "{}") return false;
    if (value == "[]") return false;
    if (value == "()") return false;
    if (value == "false") return false;
    return true;
}

bool GraphBlock::getPropertyPreview(const QString &key) const
{
    auto it = _impl->propertiesPreview.find(key);
    if (it == _impl->propertiesPreview.end()) return true; //default is ON
    if (it->second == "enable") return true;
    if (it->second == "disable") return false;
    if (it->second == "valid") return isValid(this->getPropertyValue(key));
    if (it->second == "invalid") return not isValid(this->getPropertyValue(key));
    if (it->second == "when")
    {
        //support the enum selection mode:
        //There is no parameter evaluation going on here.
        //A particular parameter, specified by the enum key,
        //is compared against a list of arguments.
        //A match means do the preview, otherwise hide it.
        if (not _impl->propertiesPreviewArgs.at(key)) return true;
        if (not _impl->propertiesPreviewKwargs.at(key)) return true;
        if (not _impl->propertiesPreviewKwargs.at(key)->has("enum")) return true;
        const auto eumParamKey = _impl->propertiesPreviewKwargs.at(key)->getValue<std::string>("enum");
        auto eumParamValue = this->getPropertyValue(QString::fromStdString(eumParamKey)).toStdString();
        for (const auto &arg : *_impl->propertiesPreviewArgs.at(key))
        {
            //note: Poco::Dynamic::Var::toString gives the JSON format
            //which means that it includes the quotes if this is a string
            if (Poco::Dynamic::Var::toString(arg) == eumParamValue) return true;
        }
        return false;
    }
    return true;
}

void GraphBlock::setPropertyPreviewMode(const QString &key, const QString &value,
    const Poco::JSON::Array::Ptr &args, const Poco::JSON::Object::Ptr &kwargs)
{
    if (_impl->propertiesPreview[key] == value) return;
    _impl->propertiesPreview[key] = value;
    _impl->propertiesPreviewArgs[key] = args;
    _impl->propertiesPreviewKwargs[key] = kwargs;
    this->markChanged();
}

void GraphBlock::setPropertyErrorMsg(const QString &key, const QString &msg)
{
    if (_impl->propertiesErrorMsg[key] == msg) return;
    _impl->propertiesErrorMsg[key] = msg;
    this->markChanged();
}

const QString &GraphBlock::getPropertyErrorMsg(const QString &key) const
{
    return _impl->propertiesErrorMsg[key];
}

void GraphBlock::setPropertyTypeStr(const QString &key, const std::string &type)
{
    if (_impl->propertiesTypeStr[key] == type) return;
    _impl->propertiesTypeStr[key] = type;
    this->markChanged();
}

const std::string &GraphBlock::getPropertyTypeStr(const QString &key) const
{
    return _impl->propertiesTypeStr[key];
}

void GraphBlock::addInputPort(const QString &portKey)
{
    _inputPorts.push_back(portKey);
    this->markChanged();
}

const QStringList &GraphBlock::getInputPorts(void) const
{
    return _inputPorts;
}

void GraphBlock::addOutputPort(const QString &portKey)
{
    _outputPorts.push_back(portKey);
    this->markChanged();
}

const QStringList &GraphBlock::getOutputPorts(void) const
{
    return _outputPorts;
}

void GraphBlock::addSlotPort(const QString &portKey)
{
    _slotPorts.push_back(portKey);
    this->markChanged();
}

const QStringList &GraphBlock::getSlotPorts(void) const
{
    return _slotPorts;
}

void GraphBlock::addSignalPort(const QString &portKey)
{
    _signalPorts.push_back(portKey);
    this->markChanged();
}

const QStringList &GraphBlock::getSignalPorts(void) const
{
    return _signalPorts;
}

void GraphBlock::setInputPortTypeStr(const QString &key, const std::string &type)
{
    if (_impl->inputPortTypeStr[key] == type) return;
    _impl->inputPortTypeStr[key] = type;
    this->markChanged();
}

const std::string &GraphBlock::getInputPortTypeStr(const QString &key) const
{
    return _impl->inputPortTypeStr[key];
}

void GraphBlock::setOutputPortTypeStr(const QString &key, const std::string &type)
{
    if (_impl->outputPortTypeStr[key] == type) return;
    _impl->outputPortTypeStr[key] = type;
    this->markChanged();
}

const std::string &GraphBlock::getOutputPortTypeStr(const QString &key) const
{
    return _impl->outputPortTypeStr[key];
}

const QString &GraphBlock::getAffinityZone(void) const
{
    return _impl->affinityZone;
}

void GraphBlock::setAffinityZone(const QString &zone)
{
    if (_impl->affinityZone == zone) return;
    _impl->affinityZone = zone;
    this->markChanged();
}

const QString &GraphBlock::getActiveEditTab(void) const
{
    return _impl->activeEditTab;
}

void GraphBlock::setActiveEditTab(const QString &name)
{
    _impl->activeEditTab = name;
}

QPainterPath GraphBlock::shape(void) const
{
    QPainterPath path;
    for (const auto &portRect : _impl->inputPortRects) path.addRect(portRect);
    for (const auto &portRect : _impl->outputPortRects) path.addRect(portRect);
    if (not this->getSignalPorts().empty()) path.addRect(_impl->signalPortRect);
    path.addRect(_impl->mainBlockRect);
    return path;
}

std::vector<GraphConnectableKey> GraphBlock::getConnectableKeys(void) const
{
    std::vector<GraphConnectableKey> keys;
    for (int i = 0; i < _inputPorts.size(); i++)
    {
        keys.push_back(GraphConnectableKey(_inputPorts[i], GRAPH_CONN_INPUT));
    }
    for (int i = 0; i < _outputPorts.size(); i++)
    {
        keys.push_back(GraphConnectableKey(_outputPorts[i], GRAPH_CONN_OUTPUT));
    }
    if (not this->getSlotPorts().empty())
    {
        keys.push_back(GraphConnectableKey("slots", GRAPH_CONN_SLOT));
    }
    if (not this->getSignalPorts().empty())
    {
        keys.push_back(GraphConnectableKey("signals", GRAPH_CONN_SIGNAL));
    }
    return keys;
}

GraphConnectableKey GraphBlock::isPointingToConnectable(const QPointF &pos) const
{
    for (int i = 0; i < _inputPorts.size(); i++)
    {
        if (_impl->inputPortRects[i].contains(pos))
            return GraphConnectableKey(_inputPorts[i], GRAPH_CONN_INPUT);
    }
    for (int i = 0; i < _outputPorts.size(); i++)
    {
        if (_impl->outputPortRects[i].contains(pos))
            return GraphConnectableKey(_outputPorts[i], GRAPH_CONN_OUTPUT);
    }
    if (not this->getSlotPorts().empty())
    {
        if (_impl->mainBlockRect.contains(pos))
            return GraphConnectableKey("slots", GRAPH_CONN_SLOT);
    }
    if (not this->getSignalPorts().empty())
    {
        if (_impl->signalPortRect.contains(pos))
            return GraphConnectableKey("signals", GRAPH_CONN_SIGNAL);
    }
    return GraphConnectableKey();
}

GraphConnectableAttrs GraphBlock::getConnectableAttrs(const GraphConnectableKey &key) const
{
    GraphConnectableAttrs attrs;
    attrs.direction = key.direction;
    attrs.rotation = this->rotation();
    if (key.direction == GRAPH_CONN_INPUT) for (int i = 0; i < _inputPorts.size(); i++)
    {
        if (key.id == _inputPorts[i])
        {
            if (_impl->inputPortPoints.size() > size_t(i)) //may not be allocated yet
                attrs.point = _impl->inputPortPoints[i];
            attrs.rotation += 180;
            return attrs;
        }
    }
    if (key.direction == GRAPH_CONN_OUTPUT) for (int i = 0; i < _outputPorts.size(); i++)
    {
        if (key.id == _outputPorts[i])
        {
            if (_impl->outputPortPoints.size() > size_t(i)) //may not be allocated yet
                attrs.point = _impl->outputPortPoints[i];
            attrs.rotation += 0;
            return attrs;
        }
    }
    if (key.direction == GRAPH_CONN_SLOT and key.id == "slots")
    {
        attrs.point = _impl->slotPortPoint;
        attrs.rotation += 270;
        return attrs;
    }
    if (key.direction == GRAPH_CONN_SIGNAL and key.id == "signals")
    {
        attrs.point = _impl->signalPortPoint;
        attrs.rotation += 90;
        return attrs;
    }
    return attrs;
}

static QStaticText makeQStaticText(const QString &s)
{
    QStaticText st(s);
    QTextOption to;
    to.setWrapMode(QTextOption::NoWrap);
    st.setTextOption(to);
    return st;
}

static QString getTextColor(const bool isOk, const QColor &bg)
{
    if (isOk) return (bg.lightnessF() > 0.5)?"black":"white";
    else return (bg.lightnessF() > 0.5)?"red":"pink";
}

void GraphBlock::renderStaticText(void)
{
    _impl->titleText = makeQStaticText(QString("<span style='color:%1;font-size:%2;'><b>%3</b></span>")
        .arg(getTextColor(this->getBlockErrorMsgs().isEmpty(), _impl->mainBlockColor))
        .arg(GraphBlockTitleFontSize)
        .arg(_impl->title.toHtmlEscaped()));

    _impl->propertiesText.clear();
    for (int i = 0; i < _properties.size(); i++)
    {
        if (not this->getPropertyPreview(_properties[i])) continue;

        //shorten text with ellipsis
        QFont font; font.setPointSize(GraphBlockPropPointWidth);
        QFontMetrics metrics(font);
        auto propText = this->getPropertyDisplayText(_properties[i]);
        propText = metrics.elidedText(propText, Qt::ElideMiddle, GraphBlockPropMaxWidthPx);

        auto text = makeQStaticText(QString("<span style='color:%1;font-size:%2;'><b>%3: </b> %4</span>")
            .arg(getTextColor(this->getPropertyErrorMsg(_properties[i]).isEmpty(), _impl->mainBlockColor))
            .arg(GraphBlockPropFontSize)
            .arg(this->getPropertyName(_properties[i]).toHtmlEscaped())
            .arg(propText.toHtmlEscaped()));
        _impl->propertiesText.push_back(text);
    }

    _impl->inputPortsText.resize(_inputPorts.size());
    for (int i = 0; i < _inputPorts.size(); i++)
    {
        _impl->inputPortsText[i] = QStaticText(QString("<span style='color:%1;font-size:%2;'>%3</span>")
            .arg(getTextColor(true, _impl->inputPortColors.at(i)))
            .arg(GraphBlockPortFontSize)
            .arg(_inputPorts[i].toHtmlEscaped()));
    }

    _impl->outputPortsText.resize(_outputPorts.size());
    for (int i = 0; i < _outputPorts.size(); i++)
    {
        _impl->outputPortsText[i] = QStaticText(QString("<span style='color:%1;font-size:%2;'>%3</span>")
            .arg(getTextColor(true, _impl->outputPortColors.at(i)))
            .arg(GraphBlockPortFontSize)
            .arg(_outputPorts[i].toHtmlEscaped()));
    }

    if (not getActionMap()["showPortNames"]->isChecked())
    {
        _impl->inputPortsText.clear();
        _impl->inputPortsText.resize(_inputPorts.size(), QStaticText(" "));
        _impl->outputPortsText.clear();
        _impl->outputPortsText.resize(_outputPorts.size(), QStaticText(" "));
    }
}

void GraphBlock::changed(void)
{
    this->markChanged();
}

static QColor generateDisabledColor(const QColor c)
{
    const QColor d(GraphBlockDisabledColor);
    if (not c.isValid()) return d;
    static const qreal alpha(GraphBlockDisabledAlphaBlend);
    return QColor(
        c.red()*alpha + d.red()*(1-alpha),
        c.green()*alpha + d.green()*(1-alpha),
        c.blue()*alpha + d.blue()*(1-alpha)
    );
}

void GraphBlock::render(QPainter &painter)
{
    //render text
    if (this->isChanged())
    {
        this->update(); //call first because this will set changed again
        this->clearChanged();

        //update colors
        auto zoneColor = dynamic_cast<AffinityZonesDock *>(getObjectMap()["affinityZonesDock"])->zoneToColor(this->getAffinityZone());
        _impl->mainBlockColor = zoneColor.isValid()?zoneColor:QColor(GraphObjectDefaultFillColor);
        if (not this->isEnabled()) _impl->mainBlockColor = generateDisabledColor(zoneColor);
        _impl->inputPortColors.resize(_inputPorts.size(), GraphObjectDefaultFillColor);
        _impl->outputPortColors.resize(_outputPorts.size(), GraphObjectDefaultFillColor);
        for (int i = 0; i < _inputPorts.size(); i++) _impl->inputPortColors[i] = typeStrToColor(this->getInputPortTypeStr(_inputPorts.at(i)));
        for (int i = 0; i < _outputPorts.size(); i++) _impl->outputPortColors[i] = typeStrToColor(this->getOutputPortTypeStr(_outputPorts.at(i)));
        this->renderStaticText();

        //connection endpoints may have moved - flag the scene for re-draw
        this->scene()->update();
    }

    //setup rotations and translations
    QTransform trans;

    //dont rotate past 180 because we just do a port flip
    //this way text only ever has 2 rotations
    const bool portFlip = this->rotation() >= 180;
    if (portFlip) painter.rotate(-180);
    if (portFlip) trans.rotate(-180);

    //calculate dimensions
    qreal inputPortsMinHeight = GraphBlockPortVOutterPad*2;
    if (_impl->inputPortsText.size() == 0) inputPortsMinHeight = 0;
    else inputPortsMinHeight += (_impl->inputPortsText.size()-1)*GraphBlockPortVGap;
    for (const auto &text : _impl->inputPortsText)
    {
        inputPortsMinHeight += text.size().height() + GraphBlockPortTextVPad*2;
    }

    qreal outputPortsMinHeight = GraphBlockPortVOutterPad*2;
    if (_impl->outputPortsText.size() == 0) outputPortsMinHeight = 0;
    else outputPortsMinHeight += (_impl->outputPortsText.size()-1)*GraphBlockPortVGap;
    for (const auto &text : _impl->outputPortsText)
    {
        outputPortsMinHeight += text.size().height() + GraphBlockPortTextVPad*2;
    }

    qreal propertiesMinHeight = 0;
    qreal propertiesMaxWidth = 0;
    for (const auto &text : _impl->propertiesText)
    {
        propertiesMinHeight += text.size().height() + GraphBlockPropTextVPad*2;
        propertiesMaxWidth = std::max<qreal>(propertiesMaxWidth, text.size().width() + GraphBlockPropTextHPad*2);
    }

    const qreal propertiesWithTitleMinHeight = GraphBlockTitleVPad + _impl->titleText.size().height() + GraphBlockTitleVPad + propertiesMinHeight;
    const qreal overallHeight = std::max(std::max(inputPortsMinHeight, outputPortsMinHeight), propertiesWithTitleMinHeight);
    const qreal overallWidth = std::max(GraphBlockTitleHPad + _impl->titleText.size().width() + GraphBlockTitleHPad, propertiesMaxWidth);

    //new dimensions for the main rectangle
    QRectF mainRect(QPointF(), QPointF(overallWidth, overallHeight));
    mainRect.moveCenter(QPointF());
    _impl->mainBlockRect = trans.mapRect(mainRect);
    auto p = mainRect.topLeft();

    //set painter for drawing the rectangles
    auto pen = QPen(QColor(GraphObjectDefaultPenColor));
    pen.setWidthF(GraphObjectBorderWidth);
    painter.setPen(pen);

    //create input ports
    const auto numInputs = this->getInputPorts().size();
    _impl->inputPortRects.resize(numInputs);
    _impl->inputPortPoints.resize(numInputs);
    qreal inPortVdelta = (overallHeight - inputPortsMinHeight)/2.0 + GraphBlockPortVOutterPad;
    for (int i = 0; i < numInputs; i++)
    {
        const auto &text = _impl->inputPortsText.at(i);
        QSizeF rectSize = text.size() + QSizeF(GraphBlockPortTextHPad*2, GraphBlockPortTextVPad*2);
        const qreal hOff = (portFlip)? overallWidth :  1-rectSize.width();
        QRectF portRect(p+QPointF(hOff, inPortVdelta), rectSize);
        inPortVdelta += rectSize.height() + GraphBlockPortVGap;
        painter.save();
        painter.setBrush(QBrush(_impl->inputPortColors.at(i)));
        if (isSelected()) painter.setPen(QColor(GraphObjectHighlightPenColor));
        painter.drawRect(portRect);
        painter.restore();
        _impl->inputPortRects[i] = trans.mapRect(portRect);

        const qreal availablePortHPad = portRect.width() - text.size().width();
        const qreal availablePortVPad = portRect.height() - text.size().height();
        painter.drawStaticText(portRect.topLeft()+QPointF(availablePortHPad/2.0, availablePortVPad/2.0), text);

        //connection point logic
        const auto connPoint = portRect.topLeft() + QPointF(portFlip?rectSize.width()+GraphObjectBorderWidth:-GraphObjectBorderWidth, rectSize.height()/2);
        _impl->inputPortPoints[i] = trans.map(connPoint);
    }

    //create output ports
    const auto numOutputs = this->getOutputPorts().size();
    _impl->outputPortRects.resize(numOutputs);
    _impl->outputPortPoints.resize(numOutputs);
    qreal outPortVdelta = (overallHeight - outputPortsMinHeight)/2.0 + GraphBlockPortVOutterPad;
    for (int i = 0; i < numOutputs; i++)
    {
        const auto &text = _impl->outputPortsText.at(i);
        QSizeF rectSize = text.size() + QSizeF(GraphBlockPortTextHPad*2+GraphBlockPortArc, GraphBlockPortTextVPad*2);
        const qreal hOff = (portFlip)? 1-rectSize.width() :  overallWidth;
        const qreal arcFix = (portFlip)? GraphBlockPortArc : -GraphBlockPortArc;
        QRectF portRect(p+QPointF(hOff+arcFix, outPortVdelta), rectSize);
        outPortVdelta += rectSize.height() + GraphBlockPortVGap;
        painter.save();
        painter.setBrush(QBrush(_impl->outputPortColors.at(i)));
        if (isSelected()) painter.setPen(QColor(GraphObjectHighlightPenColor));
        painter.drawRoundedRect(portRect, GraphBlockPortArc, GraphBlockPortArc);
        painter.restore();
        _impl->outputPortRects[i] = trans.mapRect(portRect);

        const qreal availablePortHPad = portRect.width() - text.size().width() + arcFix;
        const qreal availablePortVPad = portRect.height() - text.size().height();
        painter.drawStaticText(portRect.topLeft()+QPointF(availablePortHPad/2.0-arcFix, availablePortVPad/2.0), text);

        //connection point logic
        const auto connPoint = portRect.topLeft() + QPointF(portFlip?-GraphObjectBorderWidth:rectSize.width()+GraphObjectBorderWidth, rectSize.height()/2);
        _impl->outputPortPoints[i] = trans.map(connPoint);
    }

    //create signals port
    if (not this->getSignalPorts().empty())
    {
        QSizeF rectSize(GraphBlockSignalPortWidth, GraphBlockSignalPortHeight+GraphBlockPortArc);
        const qreal vOff = (portFlip)? 1-rectSize.height() : overallHeight;
        const qreal arcFix = (portFlip)? GraphBlockPortArc : -GraphBlockPortArc;
        QRectF portRect(p+QPointF(mainRect.width()/2-rectSize.width()/2, vOff + arcFix), rectSize);

        painter.save();
        painter.setBrush(QBrush(_impl->mainBlockColor));
        if (isSelected()) painter.setPen(QColor(GraphObjectHighlightPenColor));
        painter.drawRoundedRect(portRect, GraphBlockPortArc, GraphBlockPortArc);
        painter.restore();

        _impl->signalPortRect = trans.mapRect(portRect);

        //connection point logic
        const auto connPoint = portRect.topLeft() + QPointF(rectSize.width()/2, portFlip?-GraphObjectBorderWidth:rectSize.height()+GraphObjectBorderWidth);
        _impl->signalPortPoint = trans.map(connPoint);
    }

    //create slots port
    if (not this->getSlotPorts().empty())
    {
        //connection point logic
        const auto connPoint = mainRect.topLeft() + QPointF(mainRect.width()/2, portFlip?mainRect.height()+GraphObjectBorderWidth:-GraphObjectBorderWidth);
        _impl->slotPortPoint = trans.map(connPoint);
    }

    //draw main body of the block
    painter.save();
    painter.setBrush(QBrush(_impl->mainBlockColor));
    if (isSelected()) painter.setPen(QColor(GraphObjectHighlightPenColor));
    painter.drawRoundedRect(mainRect, GraphBlockMainArc, GraphBlockMainArc);
    painter.restore();

    //create title
    const qreal availableTitleHPad = overallWidth-_impl->titleText.size().width();
    painter.drawStaticText(p+QPointF(availableTitleHPad/2.0, GraphBlockTitleVPad), _impl->titleText);

    //create params
    qreal propVdelta = GraphBlockTitleVPad + _impl->titleText.size().height() + GraphBlockTitleVPad;
    for (const auto &text : _impl->propertiesText)
    {
        painter.drawStaticText(p+QPointF(GraphBlockPropTextHPad, propVdelta), text);
        propVdelta += GraphBlockPropTextVPad + text.size().height() + GraphBlockPropTextVPad;
    }
}

Poco::JSON::Object::Ptr GraphBlock::serialize(void) const
{
    auto obj = GraphObject::serialize();
    obj->set("what", std::string("Block"));
    obj->set("path", this->getBlockDescPath());
    obj->set("affinityZone", this->getAffinityZone().toStdString());
    if (not this->getActiveEditTab().isEmpty())
    {
        obj->set("activeEditTab", this->getActiveEditTab().toStdString());
    }

    Poco::JSON::Array jPropsObj;
    for (const auto &propKey : this->getProperties())
    {
        Poco::JSON::Object jPropObj;
        jPropObj.set("key", propKey.toStdString());
        jPropObj.set("value", this->getPropertyValue(propKey).toStdString());
        jPropsObj.add(jPropObj);
    }
    obj->set("properties", jPropsObj);

    if (_impl->inputDesc) obj->set("inputDesc", _impl->inputDesc);
    if (_impl->outputDesc) obj->set("outputDesc", _impl->outputDesc);
    return obj;
}

void GraphBlock::deserialize(Poco::JSON::Object::Ptr obj)
{
    auto path = obj->getValue<std::string>("path");
    auto properties = obj->getArray("properties");

    //init the block with the description
    auto blockDesc = getBlockDescFromPath(path);
    if (not blockDesc) throw Pothos::Exception("GraphBlock::deserialize()", "cant find block factory with path: '"+path+"'");
    this->setBlockDesc(blockDesc);

    if (obj->has("affinityZone")) this->setAffinityZone(
        QString::fromStdString(obj->getValue<std::string>("affinityZone")));

    if (obj->has("activeEditTab")) this->setActiveEditTab(
        QString::fromStdString(obj->getValue<std::string>("activeEditTab")));

    assert(properties);
    for (size_t i = 0; i < properties->size(); i++)
    {
        const auto jPropObj = properties->getObject(i);
        const auto propKey = QString::fromStdString(jPropObj->getValue<std::string>("key"));
        this->setPropertyValue(propKey, QString::fromStdString(jPropObj->getValue<std::string>("value")));
    }

    //load port description and init from it -- in the case eval fails
    if (obj->isArray("inputDesc")) this->setInputPortDesc(obj->getArray("inputDesc"));
    if (obj->isArray("outputDesc")) this->setOutputPortDesc(obj->getArray("outputDesc"));

    GraphObject::deserialize(obj);
}
