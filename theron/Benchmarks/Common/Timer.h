// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.
#ifndef THERON_BENCHMARKS_COMMON_TIMER_H
#define THERON_BENCHMARKS_COMMON_TIMER_H


#include <Theron/Defines.h>


#if THERON_WINDOWS
#include <windows.h>
#elif THERON_GCC
#include <sys/time.h>
#endif


// Simple timer class.
class Timer
{
public:

    Timer() : mSupported(false)
    {
#if THERON_WINDOWS
        // Read the counter frequency (in Hz) and an initial counter value.
        if (QueryPerformanceFrequency(&mTicksPerSecond) && QueryPerformanceCounter(&mCounterStartValue))
        {
            mSupported = true;
        }
#elif THERON_GCC
        mSupported = true;
#endif
    }

    void Start()
    {
#if THERON_WINDOWS
        QueryPerformanceCounter(&mCounterStartValue);
#elif THERON_GCC
        gettimeofday(&t1, NULL);
#endif
    }

    void Stop()
    {
#if THERON_WINDOWS
        QueryPerformanceCounter(&mCounterEndValue);
#elif THERON_GCC
        gettimeofday(&t2, NULL);
#endif
    }

    bool Supported() const
    {
        return mSupported;
    }

    float Seconds() const
    {
#if THERON_WINDOWS
        const float elapsedTicks(static_cast<float>(mCounterEndValue.QuadPart - mCounterStartValue.QuadPart));
        const float ticksPerSecond(static_cast<float>(mTicksPerSecond.QuadPart));
        return (elapsedTicks / ticksPerSecond);
#elif THERON_GCC
        return (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) * 1e-6f;
#else
        return 0.0f;
#endif
    }

private:

    Timer(const Timer &other);
    Timer &operator=(const Timer &other);

    bool mSupported;

#if THERON_WINDOWS
    LARGE_INTEGER mTicksPerSecond;
    LARGE_INTEGER mCounterStartValue;
    LARGE_INTEGER mCounterEndValue;
#elif THERON_GCC
    timeval t1, t2;
#endif

};


#endif // THERON_BENCHMARKS_COMMON_TIMER_H

