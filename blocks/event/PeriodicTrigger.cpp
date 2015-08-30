// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <chrono>
#include <thread>
#include <iostream>
#include <algorithm> //min/max

/***********************************************************************
 * |PothosDoc Periodic Trigger
 *
 * The periodic trigger block emits a signal named "triggered" at the specified interval.
 *
 * |category /Event
 *
 * |param rate[Trigger Rate] The rate of triggers per second
 * |default 1.0
 *
 * |param args Arguments to pass into the triggered signal.
 * |default []
 * |preview valid
 *
 * |factory /blocks/periodic_trigger()
 * |setter setRate(rate)
 * |setter setArgs(args)
 **********************************************************************/
class PeriodicTrigger : public Pothos::Block
{
public:
    static Block *make(void)
    {
        return new PeriodicTrigger();
    }

    PeriodicTrigger(void):
        _rate(1.0)
    {
        this->registerSignal("triggered");
        this->registerCall(this, POTHOS_FCN_TUPLE(PeriodicTrigger, setRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(PeriodicTrigger, getRate));
        this->registerCall(this, POTHOS_FCN_TUPLE(PeriodicTrigger, setArgs));
        this->registerCall(this, POTHOS_FCN_TUPLE(PeriodicTrigger, getArgs));
    }

    void setRate(const double rate)
    {
        _rate = rate;
    }

    double getRate(void) const
    {
        return _rate;
    }

    void setArgs(const Pothos::ObjectVector &args)
    {
        _args = args;
    }

    Pothos::ObjectVector getArgs(void) const
    {
        return _args;
    }

    void activate(void)
    {
        _nextTrigger = std::chrono::high_resolution_clock::now();
        this->incrementNext();
    }

    void incrementNext(void)
    {
        const auto tps = std::chrono::nanoseconds((long long)(1e9/_rate));
        _nextTrigger += std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(tps);
    }

    void work(void)
    {
        auto currentTime = std::chrono::high_resolution_clock::now();

        if (currentTime > _nextTrigger)
        {
            this->opaqueCallMethod("triggered", _args.data(), _args.size());
            this->incrementNext();
        }
        else
        {
            const auto maxSleepTime = std::chrono::nanoseconds(this->workInfo().maxTimeoutNs);
            const std::chrono::nanoseconds deltaNextTrig(_nextTrigger-currentTime);
            std::this_thread::sleep_for(std::min(maxSleepTime, deltaNextTrig));
        }

        return this->yield();
    }

private:
    double _rate;
    Pothos::ObjectVector _args;
    std::chrono::high_resolution_clock::time_point _nextTrigger;
};

static Pothos::BlockRegistry registerPeriodicTrigger(
    "/blocks/periodic_trigger", &PeriodicTrigger::make);
