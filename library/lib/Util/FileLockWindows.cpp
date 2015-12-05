// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/FileLock.hpp>
#include <Pothos/Exception.hpp>

#include <windows.h>
#include <cstring> //memset
#include <iostream>

struct Pothos::Util::FileLock::Impl
{
    Impl(void):
        handle(INVALID_HANDLE_VALUE),
        locked(false)
    {
        std::memset(&overlapped, 0, sizeof(overlapped));
    }
    HANDLE handle;
    OVERLAPPED overlapped;
    bool locked;
};

Pothos::Util::FileLock::FileLock(const std::string &filePath):
    _impl(new Impl())
{
    std::cout << "file lock " << filePath << std::endl;
    _impl->handle = CreateFile(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (_impl->handle == INVALID_HANDLE_VALUE) throw Pothos::RuntimeException("Pothos::Util::FileLock("+filePath+")", std::to_string(GetLastError()));
    std::cout << "handle = " << int(_impl->handle) << std::endl;
}

Pothos::Util::FileLock::~FileLock(void)
{
    std::cout << "close" << std::endl;
    if (_impl->locked) this->unlock();
    CloseHandle(_impl->handle);
}

void Pothos::Util::FileLock::lock(void)
{
    std::cout << "lock" << std::endl;
    if (_impl->locked) return;
    std::cout << "lock cont" << std::endl;
    LARGE_INTEGER fileSize;
    GetFileSizeEx(_impl->handle, &fileSize);
    BOOL ret = LockFileEx(
        _impl->handle,
        LOCKFILE_EXCLUSIVE_LOCK,
        0,
        fileSize.LowPart,
        fileSize.HighPart,
        &_impl->overlapped);
    std::cout << "ret " << ret << std::endl;
    WaitForSingleObject(_impl->handle, INFINITE);
    if (ret != TRUE) throw Pothos::RuntimeException("Pothos::Util::FileLock::lock()", std::to_string(GetLastError()));
    _impl->locked = true;
    std::cout << "locked" << std::endl;
}

void Pothos::Util::FileLock::unlock(void)
{
    std::cout << "unlock" << std::endl;
    if (not _impl->locked) return;
    LARGE_INTEGER fileSize;
    GetFileSizeEx(_impl->handle, &fileSize);
    BOOL ret = UnlockFileEx(
        _impl->handle,
        0,
        fileSize.LowPart,
        fileSize.HighPart,
        &_impl->overlapped);
    if (ret != TRUE) throw Pothos::RuntimeException("Pothos::Util::FileLock::unlock()", std::to_string(GetLastError()));
    _impl->locked = false;
    std::cout << "unlocked" << std::endl;
}

