// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "WaveMonitorDisplay.hpp"
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <map>

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
 * |preview disable
 *
 * |param sampleRate[Sample Rate] The rate of the input elements.
 * |default 1e6
 * |units samples/sec
 *
 * |param numPoints[Num Points] The number of points per plot capture.
 * |default 1024
 * |preview disable
 *
 * |param align[Alignment] Ensure that multiple channels are aligned.
 * All channels must have matching sample rates when alignment is enabled.
 * |default false
 * |option [Disable] false
 * |option [Enable] true
 * |preview disable
 *
 * |param autoScale[Auto-Scale] Enable automatic scaling for the vertical axis.
 * |default true
 * |option [Auto scale] true
 * |option [Use limits] false
 * |preview disable
 * |tab Axis
 *
 * |param yRange[Y-Axis Range] The minimum and maximum values for the Y-Axis.
 * When auto scale is off, this parameter controls the vertical axis.
 * |default [-1.0, 1.0]
 * |preview disable
 * |tab Axis
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
 * |param triggerSource[Source] Which input channel to monitor for trigger events.
 * |default 0
 * |widget SpinBox(minimum=0)
 * |preview disable
 * |tab Trigger
 *
 * |param triggerWindows[Windows] The number of trigger windows per output event.
 * A single output event can be composed of multiple back-to-back trigger windows.
 * |default 1
 * |widget SpinBox(minimum=1)
 * |preview disable
 * |tab Trigger
 *
 * |param triggerHoldOff[Hold Off] Hold-off subsequent trigger events for this many samples.
 * After a trigger event occurs, <em>hold off</em> disables trigger search until
 * the specified number of samples has been consumed on all input ports.
 * Hold-off is most useful when multiple trigger windows are used.
 * |units samples
 * |default 0
 * |preview disable
 * |tab Trigger
 *
 * |param triggerSlope[Slope] The required slope of the trigger detection.
 * <ul>
 * <li>Positive slope means that the trigger will be activated when the level is rises above the specified trigger level.</li>
 * <li>Negative slope means that the trigger will be activated when the level is falls below the specified trigger level.</li>
 * <li>Level means that the trigger will be activated when the trigger level is detected, regardless of the slope.</li>
 * </ul>
 * |default "POS"
 * |option [Positive] "POS"
 * |option [Negative] "NEG"
 * |option [Level] "LEVEL"
 * |preview disable
 * |tab Trigger
 *
 * |param triggerMode [Mode] The operational mode of the triggering system.
 * <ul>
 * <li>In automatic mode, the trigger event is forced by timer if none occurs.</li>
 * <li>In normal mode, samples are only forwarded when a trigger event occurs.</li>
 * <li>In periodic mode, there is no trigger search, the trigger event is forced by timer.</li>
 * <li>In disabled mode, trigger search is disabled and samples are not forwarded.</li>
 * </ul>
 * |default "AUTOMATIC"
 * |option [Automatic] "AUTOMATIC"
 * |option [Normal] "NORMAL"
 * |option [Periodic] "PERIODIC"
 * |option [Disabled] "DISABLED"
 * |preview disable
 * |tab Trigger
 *
 * |param triggerLevel [Level] The value of the input required for a trigger event.
 * |default 0.0
 * |widget DoubleSpinBox()
 * |preview disable
 * |tab Trigger
 *
 * |param triggerPosition [Position] The offset in samples before the trigger event.
 * When the samples are forwarded to the output,
 * the trigger event occurs <em>position</em> number of samples into the array.
 * |units samples
 * |default 0
 * |widget SpinBox(minimum=0)
 * |preview disable
 * |tab Trigger
 *
 * |param label0[Ch0 Label] The display label for channel 0.
 * |default ""
 * |widget StringEntry()
 * |preview disable
 * |tab Channels
 *
 * |param style0[Ch0 Style] The curve style for channel 0.
 * |default "LINE"
 * |option [Line] "LINE"
 * |option [Dash] "DASH"
 * |option [Dots] "DOTS"
 * |preview disable
 * |tab Channels
 *
 * |param label1[Ch1 Label] The display label for channel 1.
 * |default ""
 * |widget StringEntry()
 * |preview disable
 * |tab Channels
 *
 * |param style1[Ch1 Style] The curve style for channel 1.
 * |default "LINE"
 * |option [Line] "LINE"
 * |option [Dash] "DASH"
 * |option [Dots] "DOTS"
 * |preview disable
 * |tab Channels
 *
 * |param label2[Ch2 Label] The display label for channel 2.
 * |default ""
 * |widget StringEntry()
 * |preview disable
 * |tab Channels
 *
 * |param style2[Ch2 Style] The curve style for channel 2.
 * |default "LINE"
 * |option [Line] "LINE"
 * |option [Dash] "DASH"
 * |option [Dots] "DOTS"
 * |preview disable
 * |tab Channels
 *
 * |param label3[Ch3 Label] The display label for channel 3.
 * |default ""
 * |widget StringEntry()
 * |preview disable
 * |tab Channels
 *
 * |param style3[Ch3 Style] The curve style for channel 3.
 * |default "LINE"
 * |option [Line] "LINE"
 * |option [Dash] "DASH"
 * |option [Dots] "DOTS"
 * |preview disable
 * |tab Channels
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
 * |setter setAutoScale(autoScale)
 * |setter setYRange(yRange)
 * |setter enableXAxis(enableXAxis)
 * |setter enableYAxis(enableYAxis)
 * |setter setYAxisTitle(yAxisTitle)
 * |setter setTriggerSource(triggerSource)
 * |setter setTriggerWindows(triggerWindows)
 * |setter setTriggerHoldOff(triggerHoldOff)
 * |setter setTriggerSlope(triggerSlope)
 * |setter setTriggerMode(triggerMode)
 * |setter setTriggerLevel(triggerLevel)
 * |setter setTriggerPosition(triggerPosition)
 * |setter setChannelLabel(0, label0)
 * |setter setChannelStyle(0, style0)
 * |setter setChannelLabel(1, label1)
 * |setter setChannelStyle(1, style1)
 * |setter setChannelLabel(2, label2)
 * |setter setChannelStyle(2, style2)
 * |setter setChannelLabel(3, label3)
 * |setter setChannelStyle(3, style3)
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
        _display->setName("Display");

        auto registry = remoteEnv->findProxy("Pothos/BlockRegistry");
        _trigger = registry.callProxy("/blocks/wave_trigger");
        _trigger.callVoid("setName", "Trigger");

        //register calls in this topology
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveMonitor, setNumInputs));

        //display setters
        _topologyToDisplaySetter["setTitle"] = "setTitle";
        _topologyToDisplaySetter["setSampleRate"] = "setSampleRate";
        _topologyToDisplaySetter["setNumPoints"] = "setNumPoints";
        _topologyToDisplaySetter["setAutoScale"] = "setAutoScale";
        _topologyToDisplaySetter["setYRange"] = "setYRange";
        _topologyToDisplaySetter["enableXAxis"] = "enableXAxis";
        _topologyToDisplaySetter["enableYAxis"] = "enableYAxis";
        _topologyToDisplaySetter["setYAxisTitle"] = "setYAxisTitle";
        _topologyToDisplaySetter["setChannelLabel"] = "setChannelLabel";
        _topologyToDisplaySetter["setChannelStyle"] = "setChannelStyle";
        _topologyToDisplaySetter["setRateLabelId"] = "setRateLabelId";

        //trigger setters
        _topologyToTriggerSetter["setDisplayRate"] = "setEventRate";
        _topologyToTriggerSetter["setNumPoints"] = "setNumPoints";
        _topologyToTriggerSetter["setAlignment"] = "setAlignment";
        _topologyToTriggerSetter["setTriggerSource"] = "setSource";
        _topologyToTriggerSetter["setTriggerWindows"] = "setNumWindows";
        _topologyToTriggerSetter["setTriggerHoldOff"] = "setHoldOff";
        _topologyToTriggerSetter["setTriggerSlope"] = "setSlope";
        _topologyToTriggerSetter["setTriggerMode"] = "setMode";
        _topologyToTriggerSetter["setTriggerLevel"] = "setLevel";
        _topologyToTriggerSetter["setTriggerPosition"] = "setPosition";

        //connect to internal display block
        for (const auto &pair : _topologyToDisplaySetter)
        {
            this->connect(this, pair.first, _display, pair.second);
        }

        //connect to the internal trigger block
        for (const auto &pair : _topologyToTriggerSetter)
        {
            this->connect(this, pair.first, _trigger, pair.second);
        }

        //connect stream ports
        this->connect(_trigger, 0, _display, 0);
    }

    Pothos::Object opaqueCallMethod(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs) const
    {
        //calls that go to the topology
        try
        {
            return Pothos::Topology::opaqueCallMethod(name, inputArgs, numArgs);
        }
        catch (const Pothos::BlockCallNotFound &){}

        bool setterCalled = false;

        //is this a display setter?
        const auto _displaySetter = _topologyToDisplaySetter.find(name);
        if (_displaySetter != _topologyToDisplaySetter.end())
        {
            _display->opaqueCallMethod(_displaySetter->second, inputArgs, numArgs);
            setterCalled = true;
        }

        //is this a trigger setter?
        const auto _triggerSetter = _topologyToTriggerSetter.find(name);
        if (_triggerSetter != _topologyToTriggerSetter.end() and numArgs == 1)
        {
            _trigger.callVoid(_triggerSetter->second, _trigger.getEnvironment()->convertObjectToProxy(inputArgs[0]));
            setterCalled = true;
        }

        if (setterCalled) return Pothos::Object();

        //forward everything else to display
        return _display->opaqueCallMethod(name, inputArgs, numArgs);
    }

    void setNumInputs(const size_t numInputs)
    {
        _trigger.callVoid("setNumPorts", numInputs);
        for (size_t i = 0; i < numInputs; i++)
        {
            this->connect(this, i, _trigger, i);
        }
    }

private:
    Pothos::Proxy _trigger;
    std::shared_ptr<WaveMonitorDisplay> _display;

    std::map<std::string, std::string> _topologyToTriggerSetter;
    std::map<std::string, std::string> _topologyToDisplaySetter;
};

/***********************************************************************
 * registration
 **********************************************************************/
static Pothos::BlockRegistry registerWaveMonitor(
    "/widgets/wave_monitor", &WaveMonitor::make);
