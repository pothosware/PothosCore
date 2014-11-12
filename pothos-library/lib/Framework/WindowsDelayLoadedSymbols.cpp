// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Config.hpp>
#include <windows.h>
#include <cassert>

/***********************************************************************
 * delay load the kernel32 library
 **********************************************************************/
static FARPROC GetKernelProcAddress(LPCSTR lpProcName)
{
    static HMODULE hKernel = LoadLibrary("kernel32.dll");
    assert(hKernel);
    return GetProcAddress(hKernel, lpProcName);
}

/***********************************************************************
 * delay load various symbols
 **********************************************************************/
BOOL DL_GetNumaAvailableMemoryNodeEx(USHORT Node, PULONGLONG AvailableBytes)
{
    typedef BOOL (WINAPI * GetNumaAvailableMemoryNodeEx_t)(USHORT, PULONGLONG);
    static auto fcn = (GetNumaAvailableMemoryNodeEx_t)GetKernelProcAddress("GetNumaAvailableMemoryNodeEx");
    if (fcn == nullptr) return false;
    return fcn(Node, AvailableBytes);
}

BOOL DL_GetNumaNodeProcessorMaskEx(USHORT Node, PGROUP_AFFINITY ProcessorMask)
{
    typedef BOOL (WINAPI * GetNumaNodeProcessorMaskEx_t)(USHORT, PGROUP_AFFINITY);
    static auto fcn = (GetNumaNodeProcessorMaskEx_t)GetKernelProcAddress("GetNumaNodeProcessorMaskEx");
    if (fcn == nullptr) return false;
    return fcn(Node, ProcessorMask);
}

LPVOID DL_VirtualAllocExNuma(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect, DWORD nndPreferred)
{
    typedef LPVOID (WINAPI * VirtualAllocExNuma_t)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD, DWORD);
    static auto fcn = (VirtualAllocExNuma_t)GetKernelProcAddress("VirtualAllocExNuma");
    if (fcn == nullptr) return nullptr;
    return fcn(hProcess, lpAddress, dwSize, flAllocationType, flProtect, nndPreferred);
}

HANDLE DL_CreateFileMappingNuma(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCTSTR lpName, DWORD nndPreferred)
{
    typedef HANDLE (WINAPI * CreateFileMappingNuma_t)(HANDLE, LPSECURITY_ATTRIBUTES, DWORD, DWORD, DWORD, LPCTSTR, DWORD);
    static auto fcn = (CreateFileMappingNuma_t)GetKernelProcAddress("CreateFileMappingNuma");
    if (fcn == nullptr) return nullptr;
    return fcn(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName, nndPreferred);
}

LPVOID DL_MapViewOfFileExNuma(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap, LPVOID lpBaseAddress, DWORD nndPreferred)
{
    typedef LPVOID (WINAPI * MapViewOfFileExNuma_t)(HANDLE, DWORD, DWORD, DWORD, SIZE_T, LPVOID, DWORD);
    static auto fcn = (MapViewOfFileExNuma_t)GetKernelProcAddress("MapViewOfFileExNuma");
    if (fcn == nullptr) return nullptr;
    return fcn(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap, lpBaseAddress, nndPreferred);
}
