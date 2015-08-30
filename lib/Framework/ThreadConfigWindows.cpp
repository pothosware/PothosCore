// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/ThreadEnvironment.hpp"
#include <windows.h>

//delay loaded symbols for windows backwards compatibility
BOOL DL_GetNumaNodeProcessorMaskEx(USHORT Node, PGROUP_AFFINITY ProcessorMask);

std::string ThreadEnvironment::setPriority(const double prio)
{
    int nPriority(THREAD_PRIORITY_NORMAL);

    if (prio > 0)
    {
        if      (prio > +0.75) nPriority = THREAD_PRIORITY_TIME_CRITICAL;
        else if (prio > +0.50) nPriority = THREAD_PRIORITY_HIGHEST;
        else if (prio > +0.25) nPriority = THREAD_PRIORITY_ABOVE_NORMAL;
        else                   nPriority = THREAD_PRIORITY_NORMAL;
    }
    else
    {
        if      (prio < -0.75) nPriority = THREAD_PRIORITY_IDLE;
        else if (prio < -0.50) nPriority = THREAD_PRIORITY_LOWEST;
        else if (prio < -0.25) nPriority = THREAD_PRIORITY_BELOW_NORMAL;
        else                   nPriority = THREAD_PRIORITY_NORMAL;
    }

    if (SetThreadPriority(GetCurrentThread(), nPriority)) return "";
    return "SetThreadPriority() fail";
}

std::string ThreadEnvironment::setCPUAffinity(const std::vector<size_t> &affinity)
{
    KAFFINITY mask(0);
    for (const auto &cpu : affinity)
    {
        mask |= (1ull << cpu);
    }
    if (SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(mask)) != 0) return "";
    return "SetThreadAffinityMask() fail";
}

std::string ThreadEnvironment::setNodeAffinity(const std::vector<size_t> &affinity)
{
    KAFFINITY mask(0);
    for (const auto &node : affinity)
    {
        GROUP_AFFINITY groupAffinity;
        if (not DL_GetNumaNodeProcessorMaskEx(static_cast<USHORT>(node), &groupAffinity)) return "GetNumaNodeProcessorMaskEx() fail";
        mask |= groupAffinity.Mask;
    }
    if (SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(mask)) != 0) return "";
    return "SetThreadAffinityMask() fail";
}
