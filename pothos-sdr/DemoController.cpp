// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Logger.h>
#include <iostream>
#include <chrono>

/***********************************************************************
 * |PothosDoc SDR Demo Controller
 *
 * Demonstration controlling the SDR source and sink blocks from the topology.
 * This block demonstrates:
 * <ul>
 * <li>setting hardware time</li>
 * <li>getting hardware time</li>
 * <li>timed frequency tuning</li>
 * <li>requesting receive bursts</li>
 * <li>parsing receive labels</li>
 * <li>sending transmit bursts</li>
 * </ul>
 *
 * |category /SDR
 *
 * |param dtype[Data Type] The data type used by the stream ports.
 * |widget DTypeChooser(float=1,cfloat=1,int=1,cint=1)
 * |default "complex_float32"
 * |preview disable
 *
 * |factory /sdr/demo_controller(dtype)
 **********************************************************************/
class DemoController : public Pothos::Block
{
public:
    DemoController(const Pothos::DType &dtype):
        _lastHardwareTimeNs(0),
        _lastRxHardwareTimeNs(0),
        _rxTimeLabelIndex(0),
        _lastKnownRxRate(1.0)
    {
        this->setupInput(0, dtype);
        this->setupOutput(0, dtype);

        this->registerCall(this, POTHOS_FCN_TUPLE(DemoController, handleHardwareTime));
        this->registerSignal("streamControl"); //connect to streamControl on source block
        this->registerSignal("setHardwareTime"); //connect to set hardware time
        this->registerSignal("setCommandTime"); //connect to set command time
        this->registerSignal("setFrequency"); //connect to set frequency
    }

    static Block *make(const Pothos::DType &dtype)
    {
        return new DemoController(dtype);
    }

    /*!
     * Handle a hardware time event.
     * We record the hardware time and when we observed this time event.
     * Both pieces of information can be used to estimate the hardware
     * time in the future to use it for scheduling bursts and commands.
     *
     * To keep the controller block up-to-data with the hardware time,
     * the handleHardwareTime() slot may be connected to the
     * "getHardwareTimeTriggered" signal from a SDR block.
     * The receive stream may provide an "rxTime" label,
     * which can be used to also update the hardware time.
     */
    void handleHardwareTime(const long long timeNs)
    {
        _hardwareTimeEvent = std::chrono::high_resolution_clock::now();
        _lastHardwareTimeNs = timeNs;
    }

    void work(void);
    void activate(void);

private:

    /*******************************************************************
     * Dealing with time - there is more than one way.
     *
     * A periodic signal from SDR source or sink block
     * can keep us up-to-date about the current hardware time.
     *
     * Or using the receive stream labels to associate the
     * hardware time with an absolute stream element index.
     ******************************************************************/

    //the last time we received a hardware time (nanoseconds)
    long long _lastHardwareTimeNs;
    //the PC timestamp when we received the hardware time
    std::chrono::high_resolution_clock::time_point _hardwareTimeEvent;

    /*!
     * Get the approximate time on the hardware at a given point in time.
     */
    long long getApproximateHardwareTime(const std::chrono::high_resolution_clock::time_point &point)
    {
        const auto deltaNs = std::chrono::duration_cast<std::chrono::nanoseconds>(point - _hardwareTimeEvent);
        return _lastHardwareTimeNs + deltaNs.count();
    }

    //the hardware timestamp from the last rx time label
    long long _lastRxHardwareTimeNs;
    //The absolute element index for when we saw the time label
    long long _rxTimeLabelIndex;
    //the last known receive sample rate provided by a label
    double _lastKnownRxRate;

    /*!
     * Get the time of a specific stream element given its absolute index.
     */
    long long getStreamElementTime(const long long index)
    {
        const auto deltaSecs = (index - _rxTimeLabelIndex)/_lastKnownRxRate;
        return _lastRxHardwareTimeNs + (deltaSecs*1e9);
    }
};

void DemoController::activate(void)
{
    //start HW time at 0 so the numbers are understandable
    this->callVoid("setHardwareTime", 0);

    //request a tiny burst from the source block
    //the work function will print when it gets the burst
    this->callVoid("streamControl", "ACTIVATE_BURST", 0, 100);
}

void DemoController::work(void)
{
    auto inputPort = this->input(0);
    auto outputPort = this->output(0);

    //this block's work routine only reacts to input data
    if (inputPort->elements() == 0) return;
    bool sawRxEnd = false;

    //Handle the input labels, check for time, and end of burst.
    for (const auto &label : inputPort->labels())
    {
        if (label.id == "rxTime")
        {
            //updating the time-stamped hardware time
            this->handleHardwareTime(label.data.convert<long long>());

            //time tracking using the absolute element count
            _lastRxHardwareTimeNs = label.data.convert<long long>();
            _rxTimeLabelIndex = inputPort->totalElements()+label.index;
        }
        else if (label.id == "rxRate")
        {
            _lastKnownRxRate = label.data.convert<double>();
            poco_notice_f1(Poco::Logger::get("DemoController"), "RX rate is %s Msps",
                std::to_string(_lastKnownRxRate/1e6));
        }
        else if (label.id == "rxEnd")
        {
            sawRxEnd = true;
        }
    }

    poco_notice_f2(Poco::Logger::get("DemoController"), "Got %s RX elements @ %s seconds",
        std::to_string(inputPort->elements()),
        std::to_string(this->getStreamElementTime(inputPort->totalElements())/1e9));

    //The user should do something meaningful with the rx buffer...
    //const auto &rxBuff = inputPort->buffer();

    //consume the entire receive buffer
    inputPort->consume(inputPort->elements());

    //Use the rx end of burst event to trigger new actions:
    if (sawRxEnd)
    {
        //perform a timed tune 0.5 seconds from the end of this burst
        const auto commandIndex = inputPort->totalElements() + inputPort->elements() + size_t(_lastKnownRxRate/2);
        const auto commandTimeNs = this->getStreamElementTime(commandIndex);
        this->callVoid("setCommandTime", commandTimeNs);
        this->callVoid("setFrequency", 1e9);
        this->callVoid("setCommandTime", 0); //clear

        //request a timed burst 1.0 seconds from the end of this burst
        const auto streamIndex = inputPort->totalElements() + inputPort->elements() + size_t(_lastKnownRxRate);
        const auto burstTimeNs = this->getStreamElementTime(streamIndex);
        this->callVoid("streamControl", "ACTIVATE_BURST_AT", burstTimeNs, 100);

        /***************************************************************
         * Transmit an output burst at the same time as the input burst:
         * The following code shows how to get access to the tx buffer,
         * and post labels for both transmit time and end of burst.
         **************************************************************/

        //the user should fill tx buffer with something meaningful...
        const auto numElems = std::min<size_t>(outputPort->elements(), 100);
        auto &txBuff = outputPort->buffer();
        std::memset(txBuff.as<void *>(), 0, numElems*outputPort->dtype().size());

        //create a time label at the first index of the buffer
        outputPort->postLabel(Pothos::Label("txTime", burstTimeNs, 0));

        //create an end label at the last index of the buffer
        outputPort->postLabel(Pothos::Label("txEnd", true, numElems-1));

        //produce num elements of the transmit buffer
        outputPort->produce(numElems);

        //Alternative custom transmit buffer option:
        //allocate a buffer: Pothos::BufferChunk txBuff(outputPort->dtype, numElems);
        //fill the buffer and create labels as usual...
        //post the buffer: outputPort->postBuffer(txBuff);
    }
}

static Pothos::BlockRegistry registerDemoController(
    "/sdr/demo_controller", &DemoController::make);
