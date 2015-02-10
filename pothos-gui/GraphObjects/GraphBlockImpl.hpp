// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphBlock.hpp"
#include <QColor>
#include <QRectF>
#include <QPointF>
#include <QStaticText>
#include <vector>
#include <map>

struct GraphBlock::Impl
{
    Impl(void)
    {
        return;
    }

    Poco::JSON::Object::Ptr blockDesc;
    Poco::JSON::Array::Ptr inputDesc;
    Poco::JSON::Array::Ptr outputDesc;
    QString affinityZone;

    QStringList blockErrorMsgs;

    QString title;
    QStaticText titleText;
    QColor mainBlockColor;

    std::vector<QStaticText> propertiesText;
    std::map<QString, QString> propertiesValues;
    std::map<QString, QString> propertiesNames;
    std::map<QString, QString> propertiesPreview;
    std::map<QString, QString> propertiesErrorMsg;
    std::map<QString, std::string> propertiesTypeStr;

    std::vector<QStaticText> inputPortsText;
    std::vector<QRectF> inputPortRects;
    std::vector<QPointF> inputPortPoints;
    std::vector<QColor> inputPortColors;
    std::map<QString, std::string> inputPortTypeStr;

    std::vector<QStaticText> outputPortsText;
    std::vector<QRectF> outputPortRects;
    std::vector<QPointF> outputPortPoints;
    std::vector<QColor> outputPortColors;
    std::map<QString, std::string> outputPortTypeStr;

    QRectF signalPortRect;
    QPointF signalPortPoint;
    QPointF slotPortPoint;

    QRectF mainBlockRect;
    QPointer<QWidget> graphWidget;
};
