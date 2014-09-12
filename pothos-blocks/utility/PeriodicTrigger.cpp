// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <chrono>
#include <thread>
#include <iostream>

/***********************************************************************
 * |PothosDoc Periodic Trigger
 *
 * The periodic trigger block emits a signal named "triggered" at the specified interval.
 *
 * |category /Utility
 * |keywords periodic trigger
 *
 * |param rate[Trigger Rate] The rate of triggers per second
 * |default 1.0
 *
 * |factory /blocks/periodic_trigger()
 * |setter setRate(rate)
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
    }

    void setRate(const double rate)
    {
        _rate = rate;
    }

    double getRate(void) const
    {
        return _rate;
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
            this->callVoid("triggered");
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
    std::chrono::high_resolution_clock::time_point _nextTrigger;
};

static Pothos::BlockRegistry registerPeriodicTrigger(
    "/blocks/periodic_trigger", &PeriodicTrigger::make);
