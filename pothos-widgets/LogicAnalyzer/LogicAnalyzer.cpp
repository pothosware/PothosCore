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
 * |param numInputs[Num Inputs] The number of input ports.
 * |default 1
 * |widget SpinBox(minimum=1)
 * |preview disable
 *
 * |param displayRate[Display Rate] How often the plotter updates.
 * |default 1.0
 * |units updates/sec
 *
 * |param sampleRate[Sample Rate] The rate of the input elements.
 * |default 1e6
 * |units samples/sec
 *
 * |param numPoints[Num Points] The number of points per plot capture.
 * |default 256
 *
 * |param label0[Ch0 Label] The display label for channel 0.
 * |default "Ch0"
 * |widget StringEntry()
 * |preview disable
 * |tab Channels
 *
 * |param base0[Ch0 Base] The numeric base for channel 0.
 * |default 10
 * |option [Binary] 2
 * |option [Octal] 8
 * |option [Decimal] 10
 * |option [Hexadecimal] 16
 * |preview disable
 * |tab Channels
 *
 * |param label1[Ch1 Label] The display label for channel 1.
 * |default "Ch1"
 * |widget StringEntry()
 * |preview disable
 * |tab Channels
 *
 * |param base1[Ch1 Base] The numeric base for channel 1.
 * |default 10
 * |option [Binary] 2
 * |option [Octal] 8
 * |option [Decimal] 10
 * |option [Hexadecimal] 16
 * |preview disable
 * |tab Channels
 *
 * |param label2[Ch2 Label] The display label for channel 2.
 * |default "Ch2"
 * |widget StringEntry()
 * |preview disable
 * |tab Channels
 *
 * |param base2[Ch2 Base] The numeric base for channel 2.
 * |default 10
 * |option [Binary] 2
 * |option [Octal] 8
 * |option [Decimal] 10
 * |option [Hexadecimal] 16
 * |preview disable
 * |tab Channels
 *
 * |param label3[Ch2 Label] The display label for channel 3.
 * |default "Ch3"
 * |widget StringEntry()
 * |preview disable
 * |tab Channels
 *
 * |param base3[Ch3 Base] The numeric base for channel 3.
 * |default 10
 * |option [Binary] 2
 * |option [Octal] 8
 * |option [Decimal] 10
 * |option [Hexadecimal] 16
 * |preview disable
 * |tab Channels
 *
 * |param xAxisMode[X Axis Mode] Choose between index or time display.
 * |option [Index] "INDEX"
 * |option [Time] "TIME"
 * |default "INDEX"
 * |preview disable
 * |tab Axis
 *
 * |param rateLabelId[Rate Label ID] Labels with this ID can be used to set the sample rate.
 * To ignore sample rate labels, set this parameter to an empty string.
 * |default "rxRate"
 * |widget StringEntry()
 * |preview disable
 * |tab Labels
 *
 * |mode graphWidget
 * |factory /widgets/logic_analyzer(remoteEnv)
 * |initializer setNumInputs(numInputs)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setNumPoints(numPoints)
 * |setter setChannelLabel(0, label0)
 * |setter setChannelBase(0, base0)
 * |setter setChannelLabel(1, label1)
 * |setter setChannelBase(1, base1)
 * |setter setChannelLabel(2, label2)
 * |setter setChannelBase(2, base2)
 * |setter setChannelLabel(3, label3)
 * |setter setChannelBase(3, base3)
 * |setter setXAxisMode(xAxisMode)
 * |setter setRateLabelId(rateLabelId)
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
        this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzer, setNumInputs));
        this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzer, setDisplayRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(LogicAnalyzer, setNumPoints));

        //connect to internal display block
        this->connect(this, "setChannelLabel", _display, "setChannelLabel");
        this->connect(this, "setChannelBase", _display, "setChannelBase");
        this->connect(this, "setXAxisMode", _display, "setXAxisMode");

        //connect to the internal snooper block
        this->connect(this, "setDisplayRate", _snooper, "setTriggerRate");
        this->connect(this, "setNumPoints", _snooper, "setChunkSize");
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

    void setNumInputs(const size_t numInputs)
    {
        _display->setName(this->getName()+"Display");
        _snooper.callVoid("setName", this->getName()+"Snooper");

        _display->setNumInputs(numInputs);
        _snooper.callVoid("setNumPorts", numInputs);
        for (size_t i = 0; i < numInputs; i++)
        {
            this->connect(this, i, _snooper, i);
            this->connect(_snooper, i, _display, i);
        }
    }

    void setDisplayRate(const double rate)
    {
        _snooper.callVoid("setTriggerRate", rate);
    }

    void setNumPoints(const size_t num)
    {
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
