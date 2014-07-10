// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/System/NumaInfo.hpp>
#include <Windows.h>

//http://msdn.microsoft.com/en-us/library/windows/desktop/aa363804%28v=vs.85%29.aspx

std::vector<Pothos::System::NumaInfo> Pothos::System::NumaInfo::get(void)
{
    std::vector<Pothos::System::NumaInfo> infoList;

    ULONG HighestNodeNumber = 0;
    if (not GetNumaHighestNodeNumber(&HighestNodeNumber)) return infoList;
    for (USHORT node = 0; node <= HighestNodeNumber; node++)
    {
        NumaInfo info;
        info.nodeNumber = node;

        ULONGLONG AvailableBytes = 0;
        if (not GetNumaAvailableMemoryNodeEx(node, &AvailableBytes)) AvailableBytes = 0;
        info.freeMemory = AvailableBytes;

        GROUP_AFFINITY ProcessorMask;
        if (GetNumaNodeProcessorMaskEx(node, &ProcessorMask))
        {
            for (int i = 0; i < sizeof(ProcessorMask.Mask) * 8; i++)
            {
                if (ProcessorMask.Mask & (static_cast<KAFFINITY>(1) << i)) info.cpus.push_back(i);
            }
        }

        infoList.push_back(info);
    }

    return infoList;
}
