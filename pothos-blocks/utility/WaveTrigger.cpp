// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <chrono>
#include <complex>
#include <iostream>
#include <algorithm> //min/max
#include <cstring> //memcpy

/***********************************************************************
 * |PothosDoc Wave Trigger
 *
 * The Wave Trigger monitors an input port for configurable trigger events.
 * When an event is detected, the trigger block forwards selected samples
 * from all input ports to the output port as a Packet object with metadata.
 *
 * The Wave Trigger is the processing back-end for the graphical Waveform Monitor,
 * however its inputs, outputs, and configuration is well documented,
 * so it can be used in a variety of other situations if needed.
 *
 * |category /Utility
 *
 * |param numPorts[Num Ports] The number of input ports.
 * |default 1
 * |widget SpinBox(minimum=1)
 * |preview disable
 *
 * |param dataPoints[Data Points] The number of elements to yield on each channel when triggered.
 * |default 1024
 * |widget SpinBox(minimum=0)
 *
 * |param alignment[Alignment] Synchronous or asynchronous multi-channel consumption pattern.
 * When in synchronous mode, work() consumes the same amount from all channels to preserve alignment.
 * When in asynchronous mode, work() consumes all available input from each channel independently.
 * |default false
 * |option [Disable] false
 * |option [Enable] true
 *
 * |param channel[Channel] Which input channel to monitor for trigger events.
 * |default 0
 * |widget SpinBox(minimum=0)
 *
 * |param sweepRate[Sweep Rate] The rate of the trigger sweep.
 * In automatic mode, this rate sets the timer that forces a trigger event.
 * Or in the case of inadequate input after a trigger event,
 * this rate acts as a timeout to flush the available samples.
 * |units events/sec
 * |default 1.0
 *
 * |param holdOff[Hold Off] Hold off on subsequent trigger events for this many samples.
 * After a trigger event occurs, <em>hold off</em> disables trigger sweeping until
 * the specified number of samples has been consumed.
 * |units samples
 * |default 1024
 *
 * |param slope[Slope] The required slope of the trigger detection.
 * <ul>
 * <li>Positive slope means that the trigger will be activated when the level is rises above the specified trigger level.</li>
 * <li>Negative slope means that the trigger will be activated when the level is falls below the specified trigger level.</li>
 * <li>Level means that the trigger will be activated when the trigger level is detected, regardless of the slope.</li>
 * </ul>
 * |default "POS"
 * |option [Positive] "POS"
 * |option [Negative] "NEG"
 * |option [Level] "LEVEL"
 *
 * |param mode [Mode] The operational mode of the triggering system.
 * <ul>
 * <li>In automatic mode, the trigger event is forced by timer if none occurs.</li>
 * <li>In normal mode, samples are only forwarded when a trigger event occurs.</li>
 * <li>In periodic mode, there is no trigger search, the trigger event is forced by timer.</li>
 * <li>In disabled mode, trigger sweeping is disabled and samples are not forwarded.</li>
 * </ul>
 * |default "AUTOMATIC"
 * |option [Automatic] "AUTOMATIC"
 * |option [Normal] "NORMAL"
 * |option [Periodic] "PERIODIC"
 * |option [Disabled] "DISABLED"
 *
 * |param level [Level] The value of the input required for a trigger event.
 * |default 0.5
 * |widget DoubleSpinBox()
 *
 * |param position [Position] The offset in samples before the trigger event.
 * When the samples are forwarded to the output,
 * the trigger event occurs <em>position</em> number of samples into the array.
 * |units samples
 * |default 128
 * |widget SpinBox(minimum=0)
 *
 * |factory /blocks/wave_trigger()
 * |initializer setNumPorts(numPorts)
 * |setter setDataPoints(dataPoints)
 * |setter setAlignment(alignment)
 * |setter setChannel(channel)
 * |setter setSweepRate(sweepRate)
 * |setter setHoldOff(holdOff)
 * |setter setSlope(slope)
 * |setter setMode(mode)
 * |setter setLevel(level)
 * |setter setPosition(position)
 **********************************************************************/
class WaveTrigger : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new WaveTrigger();
    }

    WaveTrigger(void):
        _dataPoints(0),
        _alignment(true),
        _holdOff(0),
        _channel(0),
        _sweepRate(1.0),
        _posSlope(false),
        _negSlope(false),
        _triggerTimerEnabled(false),
        _triggerSearchEnabled(false),
        _level(0.0),
        _position(0)
    {
        this->setupInput(0);
        this->setupOutput(0);

        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setNumPorts));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setDataPoints));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getDataPoints));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setAlignment));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getAlignment));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setHoldOff));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getHoldOff));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setChannel));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getChannel));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setSweepRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getSweepRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setSlope));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getSlope));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setLevel));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getLevel));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setPosition));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getPosition));

        //initialization
        this->setDataPoints(1024);
        this->setAlignment(true);
        this->setHoldOff(1024);
        this->setChannel(0);
        this->setSweepRate(1.0);
        this->setSlope("POS");
        this->setMode("AUTOMATIC");
        this->setLevel(0.5);
        this->setPosition(128);
    }

    void setNumPorts(const size_t numPorts)
    {
        for (size_t i = this->inputs().size(); i < numPorts; i++) this->setupInput(i);
    }

    void setDataPoints(const size_t numElems)
    {
        if (numElems == 0) throw Pothos::InvalidArgumentException("WaveTrigger::setDataPoints()", "num data points must be positive");
        _dataPoints = numElems;
    }

    size_t getDataPoints(void) const
    {
        return _dataPoints;
    }

    void setAlignment(const bool enabled)
    {
        _alignment = enabled;
    }

    bool getAlignment(void) const
    {
        return _alignment;
    }

    void setHoldOff(const size_t holdOff)
    {
        _holdOff = holdOff;

        //clip to new hold off when in hold off state
        _holdOffRemaining = std::min(_holdOffRemaining, _holdOff);
    }

    bool getHoldOff(void) const
    {
        return _holdOff;
    }

    void setChannel(const size_t channel)
    {
        if (channel >= this->inputs().size()) throw Pothos::InvalidArgumentException("WaveTrigger::setChannel()", "channel out of range");
        _channel = channel;
    }

    bool getChannel(void) const
    {
        return _channel;
    }

    void setSweepRate(const double rate)
    {
        if (rate <= 0.0) throw Pothos::InvalidArgumentException("WaveTrigger::setSweepRate()", "sweep rate must be positive");
        _sweepRate = rate;
        _sweepTimeDelta = std::chrono::nanoseconds((long long)(1e9/_sweepRate));
    }

    double getSweepRate(void) const
    {
        return _sweepRate;
    }

    void setSlope(const std::string &slope)
    {
        if (slope == "POS")
        {
            _posSlope = true;
            _negSlope = false;
        }
        else if (slope == "NEG")
        {
            _posSlope = false;
            _negSlope = true;
        }
        else if (slope == "LEVEL")
        {
            _posSlope = true;
            _negSlope = true;
        }
        else
        {
            throw Pothos::InvalidArgumentException("WaveTrigger::setSlope("+slope+")", "unknown slope setting");
        }
        _slopeStr = slope;
    }

    std::string getSlope(void) const
    {
        return _slopeStr;
    }

    void setMode(const std::string &mode)
    {
        if (mode == "AUTOMATIC")
        {
            _triggerTimerEnabled = true;
            _triggerSearchEnabled = true;
        }
        else if (mode == "NORMAL")
        {
            _triggerTimerEnabled = false;
            _triggerSearchEnabled = true;
        }
        else if (mode == "PERIODIC")
        {
            _triggerTimerEnabled = true;
            _triggerSearchEnabled = false;
        }
        else if (mode == "DISABLED")
        {
            _triggerTimerEnabled = false;
            _triggerSearchEnabled = false;
        }
        else
        {
            throw Pothos::InvalidArgumentException("WaveTrigger::setMode("+mode+")", "unknown mode setting");
        }
        _modeStr = mode;
    }

    std::string getMode(void) const
    {
        return _modeStr;
    }

    void setLevel(const double level)
    {
        _level = level;
    }

    double getLevel(void) const
    {
        return _level;
    }

    void setPosition(const size_t position)
    {
        _position = position;
    }

    size_t getPosition(void) const
    {
        return _position;
    }

    void activate(void)
    {
        //reset state
        _forwardDataPoints = false;
        _holdOffRemaining = 0;

        //its like we just triggered
        _lastTriggerTime = std::chrono::high_resolution_clock::now();
    }

    void deactivate(void)
    {
        return;
    }

    void work(void);

    void propagateLabels(const Pothos::InputPort *port)
    {
        //We dont propagate labels...
        //but they are forwarded as messages,
        //where the upstream block may interpret some labels.
        //as inline configuration such as sample rate.
        auto outPort = this->output(0);
        for (const auto &label : port->labels())
        {
            outPort->postMessage(label);
        }
    }

    //! always use a circular buffer to avoid discontinuity over sliding window
    Pothos::BufferManager::Sptr getInputBufferManager(const std::string &, const std::string &)
    {
        return Pothos::BufferManager::make("circular");
    }

private:

    bool searchTriggerPointReal(const Pothos::BufferChunk &buff, const size_t numElems, double &pos);

    bool searchTriggerPointComplex(const Pothos::BufferChunk &buff, const size_t numElems,  double &pos);

    void sweepWork(void);

    //configuration settings
    size_t _dataPoints;
    bool _alignment;
    size_t _holdOff;
    size_t _channel;
    double _sweepRate;
    std::chrono::high_resolution_clock::duration _sweepTimeDelta;
    std::string _slopeStr;
    bool _posSlope;
    bool _negSlope;
    std::string _modeStr;
    bool _triggerTimerEnabled;
    bool _triggerSearchEnabled;
    double _level;
    size_t _position;

    //state tracking
    bool _triggerEventFromTimer;
    bool _forwardDataPoints;
    size_t _holdOffRemaining;
    double _triggerEventOffset;
    std::chrono::high_resolution_clock::time_point _lastTriggerTime;
};

/***********************************************************************
 * Entry point for work implementation
 **********************************************************************/
void WaveTrigger::work(void)
{
    auto outPort = this->output(0);

    //forward messages and packets
    for (auto port : this->inputs())
    {
        while (port->hasMessage())
        {
            auto msg = port->popMessage();
            if (msg.type() == typeid(Pothos::Packet))
            {
                auto pkt = msg.extract<Pothos::Packet>();
                pkt.metadata["index"] = Pothos::Object(port->index());
                outPort->postMessage(pkt);
            }
            else
            {
                outPort->postMessage(msg);
            }
        }
    }

    //trigger search mode
    if (not _forwardDataPoints) return this->sweepWork();

    //ensure the required number of elements is on each input
    for (auto port : this->inputs())
    {
        const auto &buff = port->buffer();
        if (buff.elements() >= _dataPoints) continue;
        port->setReserve(_dataPoints*buff.dtype.size());
        return;
    }

    //forward a packet for each port
    for (auto port : this->inputs())
    {
        //truncate buffer to the requested number of points
        const auto &buff = port->buffer();
        Pothos::Packet packet;
        packet.payload = buff;
        packet.payload.length = _dataPoints*buff.dtype.size();

        //append new labels
        for (const auto &inLabel : port->labels())
        {
            auto label = inLabel.toAdjusted(1, buff.dtype.size()); //bytes to elements
            if (label.index >= packet.payload.elements()) break;
            packet.labels.push_back(label);
        }

        //set metadata
        packet.metadata["index"] = Pothos::Object(port->index());
        packet.metadata["offset"] = Pothos::Object(_triggerEventOffset);

        //if the trigger point was found, record this in the metadata
        if (not _triggerEventFromTimer and size_t(port->index()) == _channel)
        {
            packet.metadata["level"] = Pothos::Object(_level);
        }

        //produce packet and consume buffer
        outPort->postMessage(packet);
        port->consume(packet.payload.length);
    }

    //reset for next sweep
    for (auto port : this->inputs()) port->setReserve(0);
    _forwardDataPoints = false;
    _holdOffRemaining = _holdOff;
}

/***********************************************************************
 * Search for the trigger point
 **********************************************************************/
void WaveTrigger::sweepWork(void)
{
    const auto trigPort = this->input(_channel);
    const auto &trigBuff = trigPort->buffer();

    //require the minimum amount leaving room for a window/history of _position
    //and an extra trailing element that isnt consumed for the slope search
    size_t numElems = trigBuff.elements();
    if (numElems <= _position+1)
    {
        //+1 after position, +1 for slope search
        trigPort->setReserve((_position+1+1)*trigBuff.dtype.size());
        return;
    }

    //calculate available elements when alignment required
    if (_alignment) for (auto port : this->inputs())
    {
        const auto &buff = port->buffer();
        numElems = std::min(numElems, buff.elements());
        if (numElems > _position+1) continue;
        //+1 after position, +1 for slope search
        port->setReserve((_position+1+1)*buff.dtype.size());
        return;
    }

    //search for the trigger point (interpolated point result)
    //for complex data, we trigger on the absolute value
    bool found = false;
    _triggerEventOffset = 0.0;
    _triggerEventFromTimer = false;
    if (_holdOffRemaining == 0 and _triggerSearchEnabled)
    {
        if (trigBuff.dtype.isComplex())
        {
            found = this->searchTriggerPointComplex(trigBuff, numElems, _triggerEventOffset);
        }

        else if (_triggerSearchEnabled and not trigBuff.dtype.isComplex())
        {
            found = this->searchTriggerPointReal(trigBuff, numElems, _triggerEventOffset);
        }
    }

    //no trigger? with the timer enabled we can force one
    if (_holdOffRemaining == 0 and not found and _triggerTimerEnabled)
    {
        found = (std::chrono::high_resolution_clock::now()-_lastTriggerTime) > _sweepTimeDelta;
        _triggerEventOffset = _position;
        _triggerEventFromTimer = true;
    }

    //determine how many elements to consume
    size_t consumeElems = 0;
    if (found)
    {
        consumeElems = size_t(_triggerEventOffset-_position);
        _triggerEventOffset -= consumeElems;
    }
    else if (_holdOffRemaining != 0)
    {
        consumeElems = std::min(numElems, _holdOffRemaining);
        _holdOffRemaining -= consumeElems;
    }
    else
    {
        consumeElems = numElems-_position-1;
    }
    /*
    std::cout << "found " << found << std::endl;
    std::cout << "numElems " << numElems << std::endl;
    std::cout << "consumeElems " << consumeElems << std::endl;
    std::cout << "_holdOffRemaining " << _holdOffRemaining << std::endl;
    //*/

    //consume from all ports, handle alignment mode
    for (auto port : this->inputs())
    {
        const auto &buff = port->buffer();
        if (_alignment or _channel == size_t(port->index()))
        {
            port->consume(consumeElems*buff.dtype.size());
        }
        else
        {
            //no alignment, consume all on non-trigger port
            port->consume(port->elements());
        }
    }

    //record the state when found
    if (found)
    {
        _forwardDataPoints = true;
        _lastTriggerTime = std::chrono::high_resolution_clock::now();
        for (auto port : this->inputs()) port->setReserve(0);
    }
}

/***********************************************************************
 * Various trigger search implementations to support real/complex
 **********************************************************************/
bool WaveTrigger::searchTriggerPointReal(const Pothos::BufferChunk &buff, const size_t numElems,  double &pos)
{
    const auto trigBuff = buff.convert(typeid(float));
    const auto p = trigBuff.as<const float *>();

    for (size_t i = _position; i < numElems-1; i++)
    {
        const auto y0 = p[i];
        const auto y1 = p[i+1];
        if ((_posSlope and y0 < _level and y1 >= _level) or
            (_negSlope and y0 > _level and y1 <= _level))
        {
            pos = i + (_level-y0)/(y1-y0);
            return true;
        }
    }
    return false;
}

bool WaveTrigger::searchTriggerPointComplex(const Pothos::BufferChunk &buff, const size_t numElems,  double &pos)
{
    const auto trigBuff = buff.convert(typeid(std::complex<float>));
    const auto p = trigBuff.as<const float *>();

    for (size_t i = _position; i < numElems-1; i++)
    {
        const auto y0 = std::abs(p[i]);
        const auto y1 = std::abs(p[i+1]);
        if ((_posSlope and y0 < _level and y1 >= _level) or
            (_negSlope and y0 > _level and y1 <= _level))
        {
            pos = i + (_level-y0)/(y1-y0);
            return true;
        }
    }
    return false;
}

static Pothos::BlockRegistry registerWaveTrigger(
    "/blocks/wave_trigger", &WaveTrigger::make);
