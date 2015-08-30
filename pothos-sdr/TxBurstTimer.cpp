// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <algorithm> //min/max
#include <iostream>
#include <thread>
#include <chrono>

/***********************************************************************
 * |PothosDoc TX Burst Timer
 *
 * Schedule a transmit burst using the most recent hardware time-stamp.
 * The burst time block consumes a stream with start and end of burst labels
 * from input port 0 and forwards the burst to output port 0 with an additional
 * <em>txTime</em> tag on the head of the burst to instruct transmit time to the SDR sink.
 *
 * <ul>
 * <li>This is a zero-copy block, buffers are passively forwarded to the output.</li>
 * <li>This type agnostic block, data type is determined from the input buffers.</li>
 * </ul>
 *
 * <h2>Hardware time</h2>
 * The hardware time should be periodically provided to the "setTime" slot.
 * The burst timer block stores the hardware time with the PC clock time,
 * and uses this information to schedule transmit times for incoming bursts.
 * Its recommended to resync the hardware time periodically
 * as it will eventually drift relative to the PC clock time.
 *
 * <h2>Resync suggestion</h2>
 * Connect the "getHardwareTimeTriggered" signal from the SDR block
 * to the "setTime" slot on the burst timer block.
 * The use a periodic trigger block and connect its "triggered"
 * signal to the "probeGetHardwareTime" slot on the SDR block.
 * The periodic trigger block will cause the SDR block to resend
 * the latest hardware time to the burst timer block on a periodic basis.
 *
 * |category /SDR
 * |keywords time burst end
 *
 * |param sampleRate[Sample Rate] The rate of the input sample stream.
 * The sample rate is used to determine the wait time between subsequent bursts.
 * |units Sps
 * |default 1e6
 *
 * |param timeDelta[Time Delta] How many seconds in the future to schedule the burst.
 * A time delta is needed to ensure that the hardware will wait for the complete
 * burst to arrive in the transmit buffers for compete contiguous transmission.
 * Too small of a time delta can cause late or fragmented bursts (depending upon the hardware implementation).
 * Too large of a time delta delays transmission and wastes usable bandwidth.
 * |units seconds
 * |default 0.1
 *
 * |param frameStartId[Frame Start ID] The label ID that marks the first element of the burst.
 * The txTime label will be produced on the exact same index as the detected frame start label.
 * |default "frameStart"
 * |widget StringEntry()
 *
 * |factory /sdr/tx_burst_timer()
 * |setter setTimeDelta(timeDelta)
 * |setter setSampleRate(sampleRate)
 * |setter setFrameStartId(frameStartId)
 **********************************************************************/
class TxBurstTimer : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new TxBurstTimer();
    }

    TxBurstTimer(void):
        _sampleRate(1.0),
        _timeDeltaNs(0),
        _lastHwTime(0),
        _lastSampleTimeNs(0)
    {
        this->setupInput(0);
        this->setupOutput(0, "", this->uid()); //unique domain because of buffer forwarding
        this->registerCall(this, POTHOS_FCN_TUPLE(TxBurstTimer, setFrameStartId));
        this->registerCall(this, POTHOS_FCN_TUPLE(TxBurstTimer, setSampleRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(TxBurstTimer, setTimeDelta));
        this->registerCall(this, POTHOS_FCN_TUPLE(TxBurstTimer, setTime));
        this->setFrameStartId("frameStart"); //initialize
        this->setSampleRate(1e6); //initialize
        this->setTimeDelta(0.1); //initialize
        this->setTime(0); //initialize
    }

    void setFrameStartId(const std::string &id)
    {
        _frameStartId = id;
    }

    void setSampleRate(const double rate)
    {
        _sampleRate = rate;
    }

    void setTimeDelta(const double delta)
    {
        _timeDeltaNs = (long long)(delta*1e9);
    }

    void setTime(const long long timeNs)
    {
        _lastHwTime = timeNs;
        _lastPcTime = std::chrono::high_resolution_clock::now();
    }

    void work(void)
    {
        auto inPort = this->input(0);
        auto outPort = this->output(0);
        if (inPort->elements() == 0) return;
        auto buff = inPort->buffer();

        //iterate through labels and respond to frame starts
        for (const auto &label : inPort->labels())
        {
            if (label.index >= inPort->elements()) break;
            if (label.id != _frameStartId) continue;

            //consume up to this point, the next work() will be index 0
            if (label.index != 0)
            {
                buff.length = label.index;
                break;
            }

            //determine transmit time
            const auto timePassed = (std::chrono::high_resolution_clock::now()-_lastPcTime);
            const auto txTimeNs = _lastHwTime+_timeDeltaNs+timePassed.count();

            //insufficient time since last burst (wait)
            if (txTimeNs < _lastSampleTimeNs)
            {
                const auto deltaNs = std::min(this->workInfo().maxTimeoutNs, _lastSampleTimeNs-txTimeNs);
                std::this_thread::sleep_for(std::chrono::nanoseconds(deltaNs));
                return this->yield();
            }

            //produce the txTime label
            outPort->postLabel(Pothos::Label("txTime", Pothos::Object(txTimeNs), label.index));
            _lastSampleTimeNs = txTimeNs;
        }

        //consume/produce
        inPort->consume(buff.length);
        outPort->postBuffer(buff);
        _lastSampleTimeNs += ((buff.elements()+1)*1e9)/_sampleRate;
    }

private:
    std::string _frameStartId;
    double _sampleRate;
    long long _timeDeltaNs;
    long long _lastHwTime;
    std::chrono::high_resolution_clock::time_point _lastPcTime;
    long long _lastSampleTimeNs;
};

static Pothos::BlockRegistry registerTxBurstTimer(
    "/sdr/tx_burst_timer", &TxBurstTimer::make);
