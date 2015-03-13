// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "LogicAnalyzerDisplay.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc Logic Analyzer
 *
 * The logic analyzer displays discrete values of a signal over time.
 *
 * |category /Widgets
 * |keywords plot logic trace wave list view
 *
 * |param title The title of the plot
 * |default "Logic Analyzer"
 * |widget StringEntry()
 * |preview valid
 *
 * |param displayRate[Display Rate] How often the plotter updates.
 * |default 10.0
 * |units updates/sec
 *
 * |param numPoints[Num Points] The number of points per plot capture.
 * |default 1024
 *
 * |mode graphWidget
 * |factory /widgets/logic_analyzer(remoteEnv)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setNumPoints(numPoints)
 **********************************************************************/
class LogicAnalyzer : public Pothos::Topology
{
public:
    static Topology *make(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        return new LogicAnalyzer(remoteEnv);
    }

    LogicAnalyzer(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        _display.reset(new LogicAnalyzerDisplay());

        auto registry = remoteEnv->findProxy("Pothos/BlockRegistry");
        _snooper = registry.callProxy("/blocks/stream_snooper");

        //register calls in this topology
        this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzer, setDisplayRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzer, setNumPoints));

        //connect to internal display block
        this->connect(this, "setTitle", _display, "setTitle");

        //connect to the internal snooper block
        this->connect(this, "setDisplayRate", _snooper, "setTriggerRate");
        this->connect(this, "setNumPoints", _snooper, "setChunkSize");

        //connect stream ports
        this->connect(this, 0, _snooper, 0);
        this->connect(_snooper, 0, _display, 0);
    }

    Pothos::Object opaqueCallMethod(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs) const
    {
        //calls that go to the topology
        try
        {
            return Pothos::Topology::opaqueCallMethod(name, inputArgs, numArgs);
        }
        catch (const Pothos::BlockCallNotFound &){}

        //forward everything else to display
        return _display->opaqueCallMethod(name, inputArgs, numArgs);
    }

    void setDisplayRate(const double rate)
    {
        _snooper.callVoid("setTriggerRate", rate);
    }

    void setNumPoints(const size_t num)
    {
        _display->setName(this->getName()+"Display");
        _snooper.callVoid("setName", this->getName()+"Snooper");

        _snooper.callVoid("setChunkSize", num);
    }

private:
    Pothos::Proxy _snooper;
    std::shared_ptr<LogicAnalyzerDisplay> _display;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerLogicAnalyzer(
    "/widgets/logic_analyzer", &LogicAnalyzer::make);
