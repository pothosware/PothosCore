// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphBlockImpl.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include <QWidget>
#include <Pothos/Proxy.hpp>
#include <Poco/Logger.h>

/***********************************************************************
 * initialize the block's properties
 **********************************************************************/
void GraphBlock::setBlockDesc(const Poco::JSON::Object::Ptr &blockDesc)
{
    assert(blockDesc);
    if (_impl->blockDesc == blockDesc) return;
    _impl->blockDesc = blockDesc;

    //extract the name or title from the description
    const auto name = blockDesc->getValue<std::string>("name");
    if (not blockDesc->has("name"))
    {
        poco_error(Poco::Logger::get("PothosGui.GraphBlock.init"), "Block missing 'name'");
        return;
    }
    this->setTitle(QString::fromStdString(name));

    //extract the params or properties from the description
    if (blockDesc->isArray("params")) for (const auto &paramObj : *blockDesc->getArray("params"))
    {
        const auto param = paramObj.extract<Poco::JSON::Object::Ptr>();
        if (not param->has("key"))
        {
            poco_error_f1(Poco::Logger::get("PothosGui.GraphBlock.init"), "Block '%s' param missing 'key'", name);
            return;
        }
        const auto key = QString::fromStdString(param->getValue<std::string>("key"));
        const auto name = QString::fromStdString(param->optValue<std::string>("name", key.toStdString()));
        this->addProperty(key);
        this->setPropertyName(key, name);

        if (param->has("default"))
        {
            this->setPropertyValue(key, QString::fromStdString(
                param->getValue<std::string>("default")));
        }
        else if (param->isArray("options") and param->getArray("options")->size() > 0)
        {
            auto opt0 = param->getArray("options")->getObject(0);
            if (not opt0->has("value"))
            {
                poco_warning_f2(Poco::Logger::get("PothosGui.GraphBlock.init"), "Block '%s' [param %s] missing 'value'", name, name.toStdString());
            }
            else this->setPropertyValue(key, QString::fromStdString(opt0->getValue<std::string>("value")));
        }

        if (param->has("preview"))
        {
            const auto prevMode = param->getValue<std::string>("preview");

            Poco::JSON::Array::Ptr args;
            if (param->has("previewArgs") and param->isArray("previewArgs"))
                args = param->getArray("previewArgs");

            Poco::JSON::Object::Ptr kwargs;
            if (param->has("previewKwargs") and param->isObject("previewKwargs"))
                kwargs = param->getObject("previewKwargs");

            this->setPropertyPreviewMode(key, QString::fromStdString(prevMode), args, kwargs);
        }
    }
}

/***********************************************************************
 * initialize the block's input ports
 **********************************************************************/
void GraphBlock::setInputPortDesc(const Poco::JSON::Array::Ptr &inputDesc)
{
    if (not inputDesc) return;
    if (_impl->inputDesc == inputDesc) return;
    _impl->inputDesc = inputDesc;

    //reload the port descriptions, clear the old first
    _inputPorts.clear();
    _slotPorts.clear();

    //reload inputs (and slots)
    for (const auto &inputPortDesc : *inputDesc)
    {
        const auto &info = inputPortDesc.extract<Poco::JSON::Object::Ptr>();
        auto portKey = QString::fromStdString(info->getValue<std::string>("name"));
        QString portAlias = portKey;
        if (info->has("alias")) portAlias = QString::fromStdString(info->getValue<std::string>("alias"));
        if (info->has("isSigSlot") and info->getValue<bool>("isSigSlot")) this->addSlotPort(portKey);
        else this->addInputPort(portKey, portAlias);
        if (info->has("dtype")) this->setInputPortTypeStr(portKey, info->getValue<std::string>("dtype"));
    }
}

/***********************************************************************
 * initialize the block's output ports
 **********************************************************************/
void GraphBlock::setOutputPortDesc(const Poco::JSON::Array::Ptr &outputDesc)
{
    if (not outputDesc) return;
    if (_impl->outputDesc == outputDesc) return;
    _impl->outputDesc = outputDesc;

    //reload the port descriptions, clear the old first
    _outputPorts.clear();
    _signalPorts.clear();

    //reload outputs (and signals)
    for (const auto &outputPortDesc : *outputDesc)
    {
        const auto &info = outputPortDesc.extract<Poco::JSON::Object::Ptr>();
        auto portKey = QString::fromStdString(info->getValue<std::string>("name"));
        QString portAlias = portKey;
        if (info->has("alias")) portAlias = QString::fromStdString(info->getValue<std::string>("alias"));
        if (info->has("isSigSlot") and info->getValue<bool>("isSigSlot")) this->addSignalPort(portKey);
        else this->addOutputPort(portKey, portAlias);
        if (info->has("dtype")) this->setOutputPortTypeStr(portKey, info->getValue<std::string>("dtype"));
    }
}
