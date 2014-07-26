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

void GraphBlock::initPropertiesFromDesc(void)
{
    const auto &blockDesc = _impl->blockDesc;

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
        auto portDesc = _blockEval.call<Poco::JSON::Object::Ptr>("inspect");

        //reload the port descriptions, clear the old first
        _inputPorts.clear();
        _outputPorts.clear();
        _slotPorts.clear();
        _signalPorts.clear();

        //reload inputs (and slots)
        for (const auto &inputPortDesc : *portDesc->getArray("inputPorts"))
        {
            const auto &info = inputPortDesc.extract<Poco::JSON::Object::Ptr>();
            auto portKey = info->getValue<std::string>("name");
            auto portName = portKey;
            if (portName.find_first_not_of("0123456789") == std::string::npos) portName = "in"+portName;
            GraphBlockPort gbp(QString::fromStdString(portKey), QString::fromStdString(portName));
            if (info->getValue<bool>("isSlot")) this->addSlotPort(gbp);
            else this->addInputPort(gbp);
        }

        //reload outputs (and signals)
        for (const auto &outputPortDesc : *portDesc->getArray("outputPorts"))
        {
            const auto &info = outputPortDesc.extract<Poco::JSON::Object::Ptr>();
            auto portKey = info->getValue<std::string>("name");
            auto portName = portKey;
            if (portName.find_first_not_of("0123456789") == std::string::npos) portName = "out"+portName;
            GraphBlockPort gbp(QString::fromStdString(portKey), QString::fromStdString(portName));
            if (info->getValue<bool>("isSignal")) this->addSignalPort(gbp);
            else this->addOutputPort(gbp);
        }
    }

    //parser errors report
    catch(const Pothos::Exception &ex)
    {
        poco_error(Poco::Logger::get("PothosGui.GraphBlock.update"), ex.displayText());
        this->setBlockErrorMsg(QString::fromStdString(ex.message()));
    }
}
