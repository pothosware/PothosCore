// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/SharedBuffer.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/Format.h>
#include <windows.h>

/***********************************************************************
 * GetSystemInfo configuration values
 **********************************************************************/
static long getpagesize (void) {
    static long g_pagesize = 0;
    if (! g_pagesize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_pagesize = system_info.dwPageSize;
    }
    return g_pagesize;
}
static long getregionsize (void) {
    static long g_regionsize = 0;
    if (! g_regionsize) {
        SYSTEM_INFO system_info;
        GetSystemInfo (&system_info);
        g_regionsize = system_info.dwAllocationGranularity;
    }
    return g_regionsize;
}

/***********************************************************************
 * generic allocation implementation
 **********************************************************************/
class GenericBufferContainer
{
public:
    GenericBufferContainer(const size_t numBytes, const long nodeAffinity):
        virtualAddr(nullptr)
    {
        const DWORD nndPreferred = (nodeAffinity == -1)? NUMA_NO_PREFERRED_NODE : nodeAffinity;

        virtualAddr = VirtualAllocExNuma(
            GetCurrentProcess(),
            NULL,
            numBytes,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_READWRITE,
            nndPreferred
        );
        if (virtualAddr == nullptr) throw Pothos::SharedBufferError(
            "Pothos::GenericBufferContainer::VirtualAllocExNuma()",
            Poco::format("error code %d", int(GetLastError())));
    }

    ~GenericBufferContainer(void)
    {
        if (virtualAddr != nullptr) VirtualFreeEx(GetCurrentProcess(), virtualAddr, 0/*must be size 0*/, MEM_RELEASE);
    }

    size_t getAddress(void) const
    {
        return size_t(virtualAddr);
    }

private:
    LPVOID virtualAddr;
};

/***********************************************************************
 * circular allocation implementation
 **********************************************************************/
class CircularBufferContainer
{
public:
    CircularBufferContainer(const size_t numBytes, const long nodeAffinity);
    ~CircularBufferContainer(void)
    {
        this->cleanup();
    }

    size_t getAddress(void) const
    {
        return size_t(virtualAddr2X);
    }

private:
    void errorOut(const std::string &what)
    {
        DWORD errorCode = GetLastError();
        this->cleanup();
        throw Pothos::SharedBufferError(
            "Pothos::CircularBufferContainer::"+what,
            Poco::format("error code %d", int(errorCode)));
    }

    void cleanup(void)
    {
        if (hMapViewOfFile1 != nullptr) UnmapViewOfFile(hMapViewOfFile1);
        hMapViewOfFile1 = nullptr;

        if (hMapViewOfFile0 != nullptr) UnmapViewOfFile(hMapViewOfFile0);
        hMapViewOfFile0 = nullptr;

        if (hFileMappingObject != nullptr) CloseHandle(hFileMappingObject);
        hFileMappingObject = nullptr;
    }

    LPVOID virtualAddr2X;
    HANDLE hFileMappingObject;
    LPVOID hMapViewOfFile0;
    LPVOID hMapViewOfFile1;
};

CircularBufferContainer::CircularBufferContainer(const size_t numBytes, const long nodeAffinity):
    virtualAddr2X(nullptr),
    hFileMappingObject(nullptr),
    hMapViewOfFile0(nullptr),
    hMapViewOfFile1(nullptr)
{
    const DWORD nndPreferred = (nodeAffinity == -1)? NUMA_NO_PREFERRED_NODE : nodeAffinity;

    /*******************************************************************
     * Step 1) get a chunk of physical memory
     ******************************************************************/
    hFileMappingObject = CreateFileMappingNuma(
        INVALID_HANDLE_VALUE,
        nullptr, //default security descripto
        PAGE_READWRITE, //rw mode
        0, numBytes, //high, low size in bytes
        nullptr, //no name
        nndPreferred);
    if (hFileMappingObject == nullptr) this->errorOut("CreateFileMappingNuma()");

    /*******************************************************************
     * Step 2) find a 2X chunk of virtual memory
     ******************************************************************/
    virtualAddr2X = VirtualAllocExNuma(
        GetCurrentProcess(),
        NULL, //determine address
        numBytes*2, //2x bytes
        MEM_RESERVE,
        PAGE_NOACCESS,
        nndPreferred);
    if (virtualAddr2X == nullptr) this->errorOut("VirtualAllocExNuma()");

    BOOL freeRet = VirtualFreeEx(
        GetCurrentProcess(),
        virtualAddr2X,
        0/*must be size 0*/,
        MEM_RELEASE);
    if (not freeRet) this->errorOut("VirtualFreeEx()");

    /*******************************************************************
     * Step 3) perform overlapping virtual mappings
     ******************************************************************/
    hMapViewOfFile0 = MapViewOfFileExNuma(
        hFileMappingObject,
        FILE_MAP_ALL_ACCESS,
        0, 0, //offset high, low
        numBytes,
        virtualAddr2X, //virtual addr
        nndPreferred);
    if (hMapViewOfFile0 == nullptr) this->errorOut("MapViewOfFileExNuma(0)");

    hMapViewOfFile1 = MapViewOfFileExNuma(
        hFileMappingObject,
        FILE_MAP_ALL_ACCESS,
        0, 0, //offset high, low
        numBytes,
        LPVOID(size_t(virtualAddr2X) + numBytes), //virtual addr
        nndPreferred);
    if (hMapViewOfFile1 == nullptr) this->errorOut("MapViewOfFileExNuma(1)");
}

/***********************************************************************
 * shared buffer factory functions
 **********************************************************************/
Pothos::SharedBuffer Pothos::SharedBuffer::make(const size_t numBytes, const long nodeAffinity)
{
    std::shared_ptr<GenericBufferContainer> container(new GenericBufferContainer(std::max<size_t>(1, numBytes), nodeAffinity));
    return SharedBuffer(container->getAddress(), numBytes, container);
}

Pothos::SharedBuffer Pothos::SharedBuffer::makeCircUnprotected(const size_t numBytesIn, const long nodeAffinity)
{
    const size_t numBytes = ((numBytesIn + getregionsize() - 1)/getregionsize())*getregionsize();
    std::shared_ptr<CircularBufferContainer> container(new CircularBufferContainer(numBytes, nodeAffinity));
    return SharedBuffer(container->getAddress(), numBytes, container);
}
