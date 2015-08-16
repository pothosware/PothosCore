// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <chrono>
#include <complex>
#include <iostream>
#include <algorithm> //min/max
#include <cstring> //memcpy
#include <set>

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
 * <h2>Packet format</h2>
 * The output packet is a Pothos::Packet with metadata, labels, and payload.
 *
 * <h3>Metadata</h3>
 * The following fields in the metadata dictionary are filled in.
 * <ul>
 * <li>"index" - which input port the packet came from</li>
 * <li>"position" - the horizontal trigger position</li>
 * <li>"level" - the configured level of the trigger</li>
 * </ul>
 *
 * <h3>Labels</h3>
 * Any labels that were associated with the buffer are included into the packet's label list.
 * In addition, when the trigger position was found (a non timer event),
 * a label with ID "T" will be added at the pre-configured trigger position,
 *
 * <h3>Payload</h3>
 * The payload is the input buffer containing the specified number of points.
 * The payload starts <em>position</em> number of samples before the trigger.
 *
 * <h2>Window operation</h2>
 * Generally, output events are scheduled at the configured event rate.
 * However, in some situations, it may be useful to see several trigger events
 * in rapid succession. The window operation mode of the wave trigger block
 * allows a configurable number of triggers to occur back-to-back.
 *
 * <h3>Window size</h3>
 * The number of samples in the payload remains unchanged,
 * however the number of samples per trigger event reduces to
 * num points/num windows.
 *
 * <h3>Hold-off delay</h3>
 * The delay between trigger windows is configured by the combination of
 * the hold-off and the trigger position. Trigger search begins after
 * the previous window + hold-off + trigger position.
 *
 * |category /Utility
 *
 * |param numPorts[Num Ports] The number of input ports.
 * |default 1
 * |widget SpinBox(minimum=1)
 * |preview disable
 *
 * |param numPoints[Num Points] The number of elements to yield on per output event.
 * |default 1024
 * |widget SpinBox(minimum=0)
 *
 * |param numWindows[Windows] The number of trigger windows per output event.
 * A single output event can be composed of multiple back-to-back trigger windows.
 * |default 1
 * |widget SpinBox(minimum=1)
 *
 * |param eventRate[Event Rate] The rate of trigger detection and output events.
 * This rate paces the activity of the trigger block to a displayable rate.
 * Input samples are discarded in-between trigger search and output events.
 * In automatic mode, this rate also sets the timer that forces a trigger event.
 * |units events/sec
 * |default 1.0
 *
 * |param alignment[Alignment] Synchronous or asynchronous multi-channel consumption pattern.
 * When in synchronous mode, work() consumes the same amount from all channels to preserve alignment.
 * When in asynchronous mode, work() consumes all available input from each channel independently.
 * |default false
 * |option [Disable] false
 * |option [Enable] true
 *
 * |param source[Source] Which input channel to monitor for trigger events.
 * |default 0
 * |widget SpinBox(minimum=0)
 *
 * |param holdOff[Hold Off] Hold-off subsequent trigger events for this many samples.
 * After a trigger event occurs, <em>hold off</em> disables trigger search until
 * the specified number of samples has been consumed on all input ports.
 * Hold-off is most useful when multiple trigger windows are used.
 * |units samples
 * |default 0
 * |preview valid
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
 * <li>In semi-automatic mode, the trigger event is forced by timer after the first window.</li>
 * <li>In normal mode, samples are only forwarded when a trigger event occurs.</li>
 * <li>In periodic mode, there is no trigger search, the trigger event is forced by timer.</li>
 * <li>In disabled mode, trigger search is disabled and samples are not forwarded.</li>
 * </ul>
 * |default "AUTOMATIC"
 * |option [Automatic] "AUTOMATIC"
 * |option [Semi-automatic] "SEMIAUTOMATIC"
 * |option [Normal] "NORMAL"
 * |option [Periodic] "PERIODIC"
 * |option [Disabled] "DISABLED"
 *
 * |param level [Level] The value of the input required for a trigger event.
 * |default 0.0
 * |widget DoubleSpinBox()
 *
 * |param position [Position] The offset in samples before the trigger event.
 * When the samples are forwarded to the output,
 * the trigger event occurs <em>position</em> number of samples into the array.
 * |units samples
 * |default 0
 * |widget SpinBox(minimum=0)
 * |preview valid
 *
 * |param labelId [Label ID] An optional label ID that causes a trigger event.
 * Rather than an input level, an associated stream label can indicate a trigger event.
 * The trigger label simply overrides the level-trigger, all other rules still apply.
 * An empty label ID disables this feature.
 * |default ""
 * |widget StringEntry()
 * |preview valid
 *
 * |factory /blocks/wave_trigger()
 * |initializer setNumPorts(numPorts)
 * |setter setNumPoints(numPoints)
 * |setter setNumWindows(numWindows)
 * |setter setEventRate(eventRate)
 * |setter setAlignment(alignment)
 * |setter setSource(source)
 * |setter setHoldOff(holdOff)
 * |setter setSlope(slope)
 * |setter setMode(mode)
 * |setter setLevel(level)
 * |setter setPosition(position)
 * |setter setLabelId(labelId)
 **********************************************************************/
class WaveTrigger : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new WaveTrigger();
    }

    WaveTrigger(void):
        _numPoints(0),
        _numWindows(1),
        _eventRate(1.0),
        _alignment(true),
        _source(0),
        _holdOff(0),
        _posSlope(false),
        _negSlope(false),
        _triggerTimerEnabled(false),
        _triggerWindowTimerEnabled(false),
        _triggerSearchEnabled(false),
        _level(0.0),
        _position(0)
    {
        this->setupInput(0);
        this->setupOutput(0);

        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setNumPorts));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setNumPoints));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getNumPoints));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setNumWindows));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getNumWindows));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setEventRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getEventRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setAlignment));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getAlignment));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setHoldOff));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getHoldOff));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setSource));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getSource));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setSlope));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getSlope));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getMode));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setLevel));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getLevel));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setPosition));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getPosition));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setLabelId));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, getLabelId));
        this->registerCall(this, POTHOS_FCN_TUPLE(WaveTrigger, setIdsList));

        //initialization
        this->setNumPoints(1024);
        this->setNumWindows(1);
        this->setEventRate(1.0);
        this->setAlignment(true);
        this->setSource(0);
        this->setHoldOff(1024);
        this->setSlope("POS");
        this->setMode("AUTOMATIC");
        this->setLevel(0.5);
        this->setPosition(128);
    }

    void setNumPorts(const size_t numPorts)
    {
        for (size_t i = this->inputs().size(); i < numPorts; i++) this->setupInput(i);
    }

    void setNumPoints(const size_t numPoints)
    {
        if (numPoints == 0) throw Pothos::InvalidArgumentException("WaveTrigger::setNumPoints()", "num points must be positive");
        _numPoints = numPoints;
    }

    size_t getNumPoints(void) const
    {
        return _numPoints;
    }

    void setNumWindows(const size_t numWindows)
    {
        if (_numWindows == 0) throw Pothos::InvalidArgumentException("WaveTrigger::setNumWindows()", "num windows must be positive");
        _numWindows = numWindows;
    }

    size_t getNumWindows(void) const
    {
        return _numWindows;
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

    void setSource(const size_t channel)
    {
        if (channel >= this->inputs().size()) throw Pothos::InvalidArgumentException("WaveTrigger::setSource()", "channel out of range");
        _source = channel;
    }

    bool getSource(void) const
    {
        return _source;
    }

    void setEventRate(const double rate)
    {
        if (rate <= 0.0) throw Pothos::InvalidArgumentException("WaveTrigger::setEventRate()", "event rate must be positive");
        _eventRate = rate;
        _eventOffDuration = std::chrono::nanoseconds((long long)(1e9/_eventRate));
        _autoForceTimeout = std::chrono::nanoseconds((long long)(1.5e9/_eventRate));
    }

    double getEventRate(void) const
    {
        return _eventRate;
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
        if (mode == "AUTOMATIC"){}
        else if (mode == "SEMIAUTOMATIC"){}
        else if (mode == "NORMAL"){}
        else if (mode == "PERIODIC"){}
        else if (mode == "DISABLED"){}
        else throw Pothos::InvalidArgumentException("WaveTrigger::setMode("+mode+")", "unknown mode setting");
        _modeStr = mode;
        _triggerWindowTimerEnabled = (mode == "SEMIAUTOMATIC");
        _triggerTimerEnabled       = (mode == "AUTOMATIC" or mode == "PERIODIC");
        _triggerSearchEnabled      = (mode == "AUTOMATIC" or mode == "SEMIAUTOMATIC" or mode == "NORMAL");
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

    void setLabelId(const std::string &id)
    {
        _labelId = id;
    }

    std::string getLabelId(void) const
    {
        return _labelId;
    }

    void setIdsList(const std::vector<std::string> &ids)
    {
        _forwardIds = std::set<std::string>(ids.begin(), ids.end());
    }

    void activate(void)
    {
        //reset state
        _pointsRemaining = 0;
        _windowsRemaining = 0;
        _holdOffRemaining = 0;
        _packets.clear();
        _packets.resize(this->inputs().size());

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
            if (_forwardIds.find(label.id) == _forwardIds.end()) continue;
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

    void triggerWork(void);

    //configuration settings
    size_t _numPoints;
    size_t _numWindows;
    double _eventRate;
    bool _alignment;
    size_t _source;
    size_t _holdOff;
    std::chrono::high_resolution_clock::duration _eventOffDuration;
    std::chrono::high_resolution_clock::duration _autoForceTimeout;
    std::string _slopeStr;
    bool _posSlope;
    bool _negSlope;
    std::string _modeStr;
    bool _triggerTimerEnabled;
    bool _triggerWindowTimerEnabled;
    bool _triggerSearchEnabled;
    double _level;
    size_t _position;
    std::string _labelId;
    std::set<std::string> _forwardIds;

    //state tracking
    bool _triggerEventFromLevel;
    size_t _pointsRemaining;
    size_t _windowsRemaining;
    size_t _holdOffRemaining;
    double _triggerEventOffset;
    std::chrono::high_resolution_clock::time_point _lastTriggerTime;
    std::vector<Pothos::Packet> _packets;
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
    if (_pointsRemaining == 0) return this->triggerWork();

    //ensure the required number of elements is on each input
    for (auto port : this->inputs())
    {
        const auto &buff = port->buffer();
        if (buff.elements() >= _pointsRemaining) continue;
        port->setReserve(_pointsRemaining*buff.dtype.size());
        return;
    }

    //flags for first and last window
    const bool firstWindow = (_windowsRemaining == _numWindows-1);
    const bool lastWindow = (_windowsRemaining == 0);

    //forward a packet for each port
    for (auto port : this->inputs())
    {
        auto &packet = _packets[port->index()];

        //truncate buffer to the requested number of points
        auto buff = port->buffer();
        buff.length = _pointsRemaining*buff.dtype.size();

        //append new labels
        for (const auto &inLabel : port->labels())
        {
            auto label = inLabel.toAdjusted(1, buff.dtype.size()); //bytes to elements
            if (label.index >= buff.elements()) break;
            label.index += packet.payload.elements();
            packet.labels.push_back(label);
        }

        //if the trigger point was found, record this in the metadata
        if (_triggerEventFromLevel and size_t(port->index()) == _source)
        {
            const auto index = _position+packet.payload.elements();
            packet.labels.push_back(Pothos::Label("T", Pothos::Object(), index));
        }

        //set metadata on the first window
        if (firstWindow)
        {
            packet.metadata["index"] = Pothos::Object(port->index());
            packet.metadata["position"] = Pothos::Object(_triggerEventOffset);
            packet.metadata["level"] = Pothos::Object(_level);
        }

        //append the buffer to the end of the packet
        if (_numWindows == 1) packet.payload = buff;
        else
        {
            if (not packet.payload)
            {
                packet.payload = Pothos::BufferChunk(buff.dtype, _numPoints);
                packet.payload.length = 0;
            }
            std::memcpy((void *)packet.payload.getEnd(), buff.as<const void *>(), buff.length);
            packet.payload.length += buff.length;
        }

        //produce the entire packet on the last window
        if (lastWindow)
        {
            outPort->postMessage(packet);
            packet = Pothos::Packet();
        }

        //consume from the input buffer
        port->consume(buff.length);
    }

    //reset for next trigger
    for (auto port : this->inputs()) port->setReserve(0);
    _pointsRemaining = 0;
    _holdOffRemaining = _holdOff;
    _lastTriggerTime = std::chrono::high_resolution_clock::now();
}

/***********************************************************************
 * Search for the trigger point
 **********************************************************************/
void WaveTrigger::triggerWork(void)
{
    const auto trigPort = this->input(_source);
    const auto &trigBuff = trigPort->buffer();

    //Search enabled when there are trigger windows left or the event time expired.
    //This lets subsequent windows trigger back to back, otherwise wait on the timer.
    //And regardless, the search is prevented by the specified hold off count.
    const auto timePassed = (std::chrono::high_resolution_clock::now()-_lastTriggerTime);
    const bool searchEnabled = ((_windowsRemaining > 0) or (timePassed > _eventOffDuration)) and (_holdOffRemaining == 0);

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
    _triggerEventOffset = _position;
    _triggerEventFromLevel = false;
    if (searchEnabled and _triggerSearchEnabled)
    {
        if (not _labelId.empty()) for (const auto &label : trigPort->labels())
        {
            if (label.id != _labelId) continue;
            const auto index = label.toAdjusted(1, trigBuff.dtype.size()).index;
            if (index < _position) continue;
            if (index >= numElems-1) break;
            found = true;
            _triggerEventOffset = index;
            break;
        }

        else if (trigBuff.dtype.isComplex())
        {
            found = this->searchTriggerPointComplex(trigBuff, numElems-1, _triggerEventOffset);
            _triggerEventFromLevel = true;
        }

        else
        {
            found = this->searchTriggerPointReal(trigBuff, numElems-1, _triggerEventOffset);
            _triggerEventFromLevel = true;
        }

        //in automatic mode, a timeout can force a trigger
        //or in semi-automatic mode on subsequent windows
        if (not found and (_triggerTimerEnabled or (_triggerWindowTimerEnabled and _windowsRemaining != 0)))
        {
            found = timePassed > _autoForceTimeout;
        }
    }

    //in periodic mode, trigger as soon as the hold is off
    else if (searchEnabled and not _triggerSearchEnabled)
    {
        found = _triggerTimerEnabled;
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
        if (_alignment or _source == size_t(port->index()))
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
        if (_windowsRemaining == 0) _windowsRemaining = _numWindows;
        _windowsRemaining--;
        _pointsRemaining = _numPoints/_numWindows;
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

    for (size_t i = _position; i < numElems; i++)
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
    const auto p = trigBuff.as<const std::complex<float> *>();

    for (size_t i = _position; i < numElems; i++)
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
