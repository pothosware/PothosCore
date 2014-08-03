// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>
#include <Poco/Timestamp.h>
#include <Poco/Thread.h>
#include <iostream>

/***********************************************************************
 * |PothosDoc Periodic Trigger
 *
 * The periodic trigger block emits a signal named "triggered" at the specified interval.
 *
 * |category /Misc
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
        this->registerCall(POTHOS_FCN_TUPLE(PeriodicTrigger, setRate));
        this->registerCall(POTHOS_FCN_TUPLE(PeriodicTrigger, getRate));
    }

    void setRate(const double rate)
    {
        _rate = rate;
    }

    double getRate(void) const
    {
        return _rate;
    }

    void work(void)
    {
        auto currentTime = Poco::Timestamp();

        if (currentTime > _nextTrigger)
        {
            this->emitSignal("triggered");
            _nextTrigger += Poco::Timestamp::TimeDiff(1e6/_rate);
        }
        else
        {
            auto maxTimeSleepMs = this->workInfo().maxTimeoutNs/1e6; //ns to ms
            auto deltaNextTrigMs = (_nextTrigger-currentTime)/1e3; //us to ms
            Poco::Thread::sleep(std::min(maxTimeSleepMs, deltaNextTrigMs));
        }

        return this->yield();
    }

private:
    double _rate;
    Poco::Timestamp _nextTrigger;
};

static Pothos::BlockRegistry registerPeriodicTrigger(
    "/blocks/periodic_trigger", &PeriodicTrigger::make);
