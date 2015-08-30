// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/SharedBuffer.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Poco/TemporaryFile.h>
#include <Poco/Format.h>
#include <cassert>
#include <fcntl.h> //open
#include <unistd.h> //close
#include <cerrno> //errno
#include <cstring> //strerror
#include <sys/mman.h> //mmap

//MAP_ANON is deprecated - this supports older headers
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#if HAVE_LIBNUMA
#include <numa.h>
#endif

/***********************************************************************
 * aligned allocator for a generic memory slab (uses new/delete)
 **********************************************************************/
#define ALIGNMENT_BYTES 64

class GenericBufferContainer
{
public:
    GenericBufferContainer(const size_t numBytes)
    {
        _mem = new char[numBytes + ALIGNMENT_BYTES - 1];
    }

    ~GenericBufferContainer(void)
    {
        delete[] _mem;
    }

    size_t getAddress(void) const
    {
        size_t memAddr = size_t(_mem);
        const size_t mod = memAddr % ALIGNMENT_BYTES;
        assert(ALIGNMENT_BYTES > mod);
        if (mod != 0) memAddr += ALIGNMENT_BYTES - mod;
        assert(memAddr >= size_t(_mem));
        return memAddr;
    }

private:
    char *_mem;
};

/***********************************************************************
 * numa allocator for an affinitized memory slab
 **********************************************************************/
class GenericBufferContainerNuma
{
public:
    GenericBufferContainerNuma(const size_t numBytes, const long nodeAffinity):
        _mem(nullptr),
        _len(numBytes)
    {
        #if HAVE_LIBNUMA
        if (numa_available() == -1) return;
        _mem = numa_alloc_onnode(numBytes, nodeAffinity);
        #endif
    }

    ~GenericBufferContainerNuma(void)
    {
        #if HAVE_LIBNUMA
        if (_mem != nullptr) numa_free(_mem, _len);
        #endif
    }

    size_t getAddress(void) const
    {
        return size_t(_mem);
    }

private:
    void *_mem;
    size_t _len;
};

/***********************************************************************
 * numa allocator for an affinitized memory slab
 **********************************************************************/
class CircularBufferContainer
{
public:
    CircularBufferContainer(const size_t numBytes);
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
        const int errnoSave = errno;
        this->cleanup();
        throw Pothos::SharedBufferError(
            "Pothos::CircularBufferContainer::"+what,
            Poco::format("errno %d - %s", errnoSave, std::string(strerror(errnoSave))));
    }

    void cleanup(void)
    {
        if (mapPtr1 != MAP_FAILED) munmap(mapPtr1, _numBytes);
        mapPtr1 = MAP_FAILED;

        if (mapPtr0 != MAP_FAILED) munmap(mapPtr0, _numBytes);
        mapPtr0 = MAP_FAILED;

        if (tmpFd >= 0)
        {
            close(tmpFd);
            unlink(tmpFile.c_str());
        }
        tmpFd = -1;
    }

    const size_t _numBytes;
    void *virtualAddr2X;
    std::string tmpFile;
    int tmpFd;
    void *mapPtr0;
    void *mapPtr1;
};

CircularBufferContainer::CircularBufferContainer(const size_t numBytes):
    _numBytes(numBytes),
    virtualAddr2X(nullptr),
    tmpFd(-1),
    mapPtr0(MAP_FAILED),
    mapPtr1(MAP_FAILED)
{
    int ret = 0;

    /*******************************************************************
     * Step 1) open a temp file for physical memory
     ******************************************************************/
    tmpFile = Poco::TemporaryFile::tempName();
    tmpFd = open(
        tmpFile.c_str(),
        O_RDWR | O_CREAT | O_EXCL,
        S_IRUSR | S_IWUSR);
    if (tmpFd < 0) this->errorOut("open("+ tmpFile +")");

    ret = ftruncate(tmpFd, numBytes*2);
    if (ret != 0) this->errorOut("ftruncate("+ tmpFile +")");

    /*******************************************************************
     * Step 2) find a 2X chunk of virtual memory
     ******************************************************************/
    virtualAddr2X = mmap(
        nullptr,
        numBytes*2,
        PROT_NONE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1, off_t(0));
    if (virtualAddr2X == MAP_FAILED) this->errorOut("mmap(2x)");

    ret = munmap(virtualAddr2X, numBytes*2);
    if (ret != 0) this->errorOut("munmap(2x)");

    /*******************************************************************
     * Step 3) perform overlapping virtual mappings
     ******************************************************************/
    mapPtr0 = mmap(
        virtualAddr2X,
        numBytes,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        tmpFd, off_t(0));
    if (mapPtr0 == MAP_FAILED) this->errorOut("mmap(0)");

    mapPtr1 = mmap(
        (void *)(size_t(virtualAddr2X) + numBytes),
        numBytes,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        tmpFd, off_t(0));
    if (mapPtr1 == MAP_FAILED) this->errorOut("mmap(1)");
}

/***********************************************************************
 * shared buffer implementation
 **********************************************************************/
Pothos::SharedBuffer Pothos::SharedBuffer::make(const size_t numBytes, const long nodeAffinity)
{
    size_t address = 0;
    std::shared_ptr<void> deleter;

    //node affinity specified, perform allocation on node
    if (nodeAffinity >= 0)
    {
        std::shared_ptr<GenericBufferContainerNuma> sharedAlloc(new GenericBufferContainerNuma(numBytes, nodeAffinity));
        address = sharedAlloc->getAddress();
        deleter = sharedAlloc;
    }

    //address is 0 when numa alloc is not run or it fails
    if (address == 0)
    {
        std::shared_ptr<GenericBufferContainer> sharedAlloc(new GenericBufferContainer(numBytes));
        address = sharedAlloc->getAddress();
        deleter = sharedAlloc;
    }

    return SharedBuffer(address, numBytes, deleter);
}

Pothos::SharedBuffer Pothos::SharedBuffer::makeCircUnprotected(const size_t numBytesIn, const long)
{
    const size_t numBytes = ((numBytesIn + getpagesize() - 1)/getpagesize())*getpagesize();
    std::shared_ptr<CircularBufferContainer> container(new CircularBufferContainer(numBytes));
    return SharedBuffer(container->getAddress(), numBytes, container);
}
