// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "GraphObjects/GraphBlockImpl.hpp"
#include "GraphEditor/GraphDraw.hpp"
#include "GraphEditor/GraphEditor.hpp"
#include "TopologyEngine/TopologyEngine.hpp"
#include <Pothos/Proxy.hpp>
#include <Pothos/Remote.hpp>
#include <Pothos/Framework.hpp>
#include <Poco/Logger.h>
#include <iostream>

/***********************************************************************
 * initialize the block's properties
 **********************************************************************/
void GraphBlock::initPropertiesFromDesc(void)
{
    const auto &blockDesc = _impl->blockDesc;
    assert(blockDesc);

    //extract the name or title from the description
    const auto name = blockDesc->get("name").convert<std::string>();
    this->setTitle(QString::fromStdString(name));

    //extract the params or properties from the description
    for (const auto &paramObj : *blockDesc->getArray("params"))
    {
        const auto param = paramObj.extract<Poco::JSON::Object::Ptr>();
        const auto key = QString::fromStdString(param->get("key").convert<std::string>());
        const auto name = QString::fromStdString(param->get("name").convert<std::string>());
        this->addProperty(GraphBlockProp(key, name));

        const auto options = param->getArray("options");
        if (param->has("default"))
        {
            this->setPropertyValue(key, QString::fromStdString(
                param->get("default").convert<std::string>()));
        }
        else if (options and options->size() > 0)
        {
            this->setPropertyValue(key, QString::fromStdString(
                options->getObject(0)->get("value").convert<std::string>()));
        }

        if (param->has("preview"))
        {
            const auto prev = param->get("preview").convert<std::string>();
            this->setPropertyPreview(key, prev == "enabled");
        }
    }
}

/***********************************************************************
 * initialize the block's input ports
 **********************************************************************/
void GraphBlock::initInputsFromDesc(void)
{
    const auto &inputDesc = _impl->inputDesc;
    if (not inputDesc) return;

    //reload the port descriptions, clear the old first
    _inputPorts.clear();
    _slotPorts.clear();

    //reload inputs (and slots)
    for (const auto &inputPortDesc : *inputDesc)
    {
        const auto &info = inputPortDesc.extract<Poco::JSON::Object::Ptr>();
        auto portKey = info->getValue<std::string>("name");
        auto portName = portKey;
        if (portName.find_first_not_of("0123456789") == std::string::npos) portName = "in"+portName;
        GraphBlockPort gbp(QString::fromStdString(portKey), QString::fromStdString(portName));
        if (info->has("isSpecial") and info->getValue<bool>("isSpecial")) this->addSlotPort(gbp);
        else this->addInputPort(gbp);
        if (info->has("dtype")) this->setInputPortTypeStr(gbp.getKey(), info->getValue<std::string>("dtype"));
    }
}

/***********************************************************************
 * initialize the block's output ports
 **********************************************************************/
void GraphBlock::initOutputsFromDesc(void)
{
    const auto &outputDesc = _impl->outputDesc;
    if (not outputDesc) return;

    //reload the port descriptions, clear the old first
    _outputPorts.clear();
    _signalPorts.clear();

    //reload outputs (and signals)
    for (const auto &outputPortDesc : *outputDesc)
    {
        const auto &info = outputPortDesc.extract<Poco::JSON::Object::Ptr>();
        auto portKey = info->getValue<std::string>("name");
        auto portName = portKey;
        if (portName.find_first_not_of("0123456789") == std::string::npos) portName = "out"+portName;
        GraphBlockPort gbp(QString::fromStdString(portKey), QString::fromStdString(portName));
        if (info->has("isSpecial") and info->getValue<bool>("isSpecial")) this->addSignalPort(gbp);
        else this->addOutputPort(gbp);
        if (info->has("dtype")) this->setOutputPortTypeStr(gbp.getKey(), info->getValue<std::string>("dtype"));
    }
}

/***********************************************************************
 * instantiate the block, check for errors, query the ports
 **********************************************************************/

//! helper to convert the port info vector into JSON for serialization of the block
static Poco::JSON::Array::Ptr portInfosToJSON(const std::vector<Pothos::PortInfo> &infos)
{
    Poco::JSON::Array::Ptr array = new Poco::JSON::Array();
    for (const auto &info : infos)
    {
        Poco::JSON::Object::Ptr portInfo = new Poco::JSON::Object();
        portInfo->set("name", info.name);
        portInfo->set("isSpecial", info.isSpecial);
        portInfo->set("size", info.dtype.size());
        portInfo->set("dtype", info.dtype.toString());
        array->add(portInfo);
    }
    return array;
}

void GraphBlock::update(void)
{
    assert(_impl->blockDesc);

    auto draw = dynamic_cast<GraphDraw *>(this->parent());
    assert(draw != nullptr);
    auto engine = draw->getGraphEditor()->getTopologyEngine();

    Pothos::ProxyEnvironment::Sptr env;
    POTHOS_EXCEPTION_TRY
    {
        env = engine->getEnvironmentFromZone(this->getAffinityZone());
    }
    POTHOS_EXCEPTION_CATCH (const Pothos::Exception &ex)
    {
        this->setBlockErrorMsg(QString::fromStdString(ex.displayText()));
        return;
    }
    auto EvalEnvironment = env->findProxy("Pothos/Util/EvalEnvironment");
    auto BlockEval = env->findProxy("Pothos/Util/BlockEval");

    auto evalEnv = EvalEnvironment.callProxy("new");
    _blockEval = BlockEval.callProxy("new", evalEnv);

    this->setBlockErrorMsg("");

    //validate the id
    if (this->getId().isEmpty())
    {
        this->setBlockErrorMsg(tr("Error: empty ID"));
    }

    //evaluate the properties
    bool hasError = false;
    for (const auto &prop : this->getProperties())
    {
        const auto val = this->getPropertyValue(prop.getKey()).toStdString();
        try
        {
            auto obj = _blockEval.callProxy("evalProperty", prop.getKey().toStdString(), val);
            this->setPropertyTypeStr(prop.getKey(), obj.call<std::string>("getTypeString"));
            this->setPropertyErrorMsg(prop.getKey(), "");
        }
        catch (const Pothos::Exception &ex)
        {
            this->setPropertyErrorMsg(prop.getKey(), QString::fromStdString(ex.message()));
            hasError = true;
        }
    }

    //property errors -- cannot continue
    if (hasError)
    {
        this->setBlockErrorMsg(tr("Error: cannot evaluate this block with property errors"));
        return;
    }

    //evaluate the block and load its port info
    try
    {
        _blockEval.callProxy("eval", this->getId().toStdString(), this->getBlockDesc());
    }

    //parser errors report
    catch(const Pothos::Exception &ex)
    {
        poco_error(Poco::Logger::get("PothosGui.GraphBlock.update"), ex.displayText());
        this->setBlockErrorMsg(QString::fromStdString(ex.message()));
    }

    //update the ports after complete evaluation
    auto block = _blockEval.callProxy("getProxyBlock");
    _impl->inputDesc = portInfosToJSON(block.call<std::vector<Pothos::PortInfo>>("inputPortInfo"));
    _impl->outputDesc = portInfosToJSON(block.call<std::vector<Pothos::PortInfo>>("outputPortInfo"));
    this->initInputsFromDesc();
    this->initOutputsFromDesc();
}
