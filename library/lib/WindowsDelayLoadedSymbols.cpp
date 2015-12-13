// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Config.hpp>
#include <windows.h>
#include <iostream>
#include <cassert>

/***********************************************************************
 * delay load the kernel32 library
 **********************************************************************/
static FARPROC GetKernelProcAddress(LPCSTR lpProcName)
{
    static HMODULE hKernel = LoadLibrary("kernel32.dll");
    assert(hKernel);
    FARPROC r = GetProcAddress(hKernel, lpProcName);
    if (not r) std::cerr << "This kernel32 does not support " << lpProcName << std::endl;
    return r;
}

/***********************************************************************
 * set error mode with non-thread safe backup
 **********************************************************************/
BOOL DL_SetThreadErrorMode(DWORD dwNewMode, LPDWORD lpOldMode)
{
    typedef BOOL (WINAPI * SetThreadErrorMode_t)(DWORD, LPDWORD);
    static auto fcn = (SetThreadErrorMode_t)GetKernelProcAddress("SetThreadErrorMode");
    if (not fcn)
    {
        *lpOldMode = SetErrorMode(dwNewMode);
        return true;
    }
    return fcn(dwNewMode, lpOldMode);
}

/***********************************************************************
 * numa information
 **********************************************************************/
BOOL DL_GetNumaAvailableMemoryNodeEx(USHORT Node, PULONGLONG AvailableBytes)
{
    typedef BOOL (WINAPI * GetNumaAvailableMemoryNodeEx_t)(USHORT, PULONGLONG);
    static auto fcn = (GetNumaAvailableMemoryNodeEx_t)GetKernelProcAddress("GetNumaAvailableMemoryNodeEx");
    if (not fcn) return false;
    return fcn(Node, AvailableBytes);
}

BOOL DL_GetNumaNodeProcessorMaskEx(USHORT Node, PGROUP_AFFINITY ProcessorMask)
{
    typedef BOOL (WINAPI * GetNumaNodeProcessorMaskEx_t)(USHORT, PGROUP_AFFINITY);
    static auto fcn = (GetNumaNodeProcessorMaskEx_t)GetKernelProcAddress("GetNumaNodeProcessorMaskEx");
    if (not fcn) return false;
    return fcn(Node, ProcessorMask);
}

/***********************************************************************
 * numa allocation -- with non-numa backup support
 **********************************************************************/
LPVOID DL_VirtualAllocExNuma(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect, DWORD nndPreferred)
{
    typedef LPVOID (WINAPI * VirtualAllocExNuma_t)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD, DWORD);
    static auto fcn = (VirtualAllocExNuma_t)GetKernelProcAddress("VirtualAllocExNuma");
    if (not fcn) return VirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
    return fcn(hProcess, lpAddress, dwSize, flAllocationType, flProtect, nndPreferred);
}

HANDLE DL_CreateFileMappingNuma(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCTSTR lpName, DWORD nndPreferred)
{
    typedef HANDLE (WINAPI * CreateFileMappingNuma_t)(HANDLE, LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD, LPCTSTR, DWORD);
    static auto fcn = (CreateFileMappingNuma_t)GetKernelProcAddress("CreateFileMappingNumaA");
    if (not fcn) return CreateFileMapping(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    return fcn(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName, nndPreferred);
}

LPVOID DL_MapViewOfFileExNuma(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap, LPVOID lpBaseAddress, DWORD nndPreferred)
{
    typedef LPVOID (WINAPI * MapViewOfFileExNuma_t)(HANDLE, DWORD, DWORD, DWORD, SIZE_T, LPVOID, DWORD);
    static auto fcn = (MapViewOfFileExNuma_t)GetKernelProcAddress("MapViewOfFileExNuma");
    if (not fcn) return MapViewOfFileEx(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap, lpBaseAddress);
    return fcn(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap, lpBaseAddress, nndPreferred);
}

/***********************************************************************
 * get processor information
 **********************************************************************/
BOOL DL_GetLogicalProcessorInformationEx(LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Buffer, PDWORD ReturnedLength)
{
    typedef BOOL (WINAPI * GetLogicalProcessorInformationEx_t)(LOGICAL_PROCESSOR_RELATIONSHIP, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX, PDWORD);
    static auto fcn = (GetLogicalProcessorInformationEx_t)GetKernelProcAddress("GetLogicalProcessorInformationEx");
    if (not fcn) return false;
    return fcn(RelationshipType, Buffer, ReturnedLength);
}
