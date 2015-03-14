// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "WaveMonitorDisplay.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>

/***********************************************************************
 * |PothosDoc Wave Monitor
 *
 * The wave monitor plot displays a live two dimensional plot of input elements vs time.
 *
 * |category /Plotters
 * |keywords time plot wave scope
 *
 * |param title The title of the plot
 * |default "Amplitude vs Time"
 * |widget StringEntry()
 * |preview valid
 *
 * |param numInputs[Num Inputs] The number of input ports.
 * |default 1
 * |widget SpinBox(minimum=1)
 * |preview disable
 *
 * |param displayRate[Display Rate] How often the plotter updates.
 * |default 10.0
 * |units updates/sec
 *
 * |param sampleRate[Sample Rate] The rate of the input elements.
 * |default 1e6
 * |units samples/sec
 *
 * |param numPoints[Num Points] The number of points per plot capture.
 * |default 1024
 *
 * |param align[Alignment] Ensure that multiple channels are aligned.
 * All channels must have matching sample rates when alignment is enabled.
 * |default false
 * |option [Disable] false
 * |option [Enable] true
 *
 * |param enableXAxis[Enable X-Axis] Show or hide the horizontal axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 * |tab Axis
 *
 * |param enableYAxis[Enable Y-Axis] Show or hide the vertical axis markers.
 * |option [Show] true
 * |option [Hide] false
 * |default true
 * |preview disable
 * |tab Axis
 *
 * |param yAxisTitle[Y-Axis Title] The title of the verical axis.
 * |default ""
 * |widget StringEntry()
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
 * |factory /widgets/wave_monitor(remoteEnv)
 * |initializer setNumInputs(numInputs)
 * |setter setTitle(title)
 * |setter setDisplayRate(displayRate)
 * |setter setSampleRate(sampleRate)
 * |setter setNumPoints(numPoints)
 * |setter setAlignment(align)
 * |setter enableXAxis(enableXAxis)
 * |setter enableYAxis(enableYAxis)
 * |setter setYAxisTitle(yAxisTitle)
 * |setter setRateLabelId(rateLabelId)
 **********************************************************************/
class WaveMonitor : public Pothos::Topology
{
public:
    static Topology *make(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        return new WaveMonitor(remoteEnv);
    }

    WaveMonitor(const Pothos::ProxyEnvironment::Sptr &remoteEnv)
    {
        _display.reset(new WaveMonitorDisplay());

        auto registry = remoteEnv->findProxy("Pothos/BlockRegistry");
        _snooper = registry.callProxy("/blocks/stream_snooper");

        //register calls in this topology
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setNumInputs));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setDisplayRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setNumPoints));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setAlignment));

        //connect to internal display block
        this->connect(this, "setTitle", _display, "setTitle");
        this->connect(this, "setSampleRate", _display, "setSampleRate");
        this->connect(this, "setNumPoints", _display, "setNumPoints");
        this->connect(this, "enableXAxis", _display, "enableXAxis");
        this->connect(this, "enableYAxis", _display, "enableYAxis");
        this->connect(this, "setYAxisTitle", _display, "setYAxisTitle");

        //connect to the internal snooper block
        this->connect(this, "setDisplayRate", _snooper, "setTriggerRate");
        this->connect(this, "setNumPoints", _snooper, "setChunkSize");
        this->connect(this, "setAlignment", _snooper, "setAlignment");
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
        _display->setNumPoints(num);
    }

    void setAlignment(const bool enabled)
    {
        _snooper.callVoid("setAlignment", enabled);
    }

private:
    Pothos::Proxy _snooper;
    std::shared_ptr<WaveMonitorDisplay> _display;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerWaveMonitor(
    "/widgets/wave_monitor", &WaveMonitor::make);
