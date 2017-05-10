// Copyright (c) 2015-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/FileLock.hpp>
#include <Pothos/Exception.hpp>

#include <windows.h>
#include <cstring> //memset

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
    _impl->handle = CreateFile(
        filePath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (_impl->handle == INVALID_HANDLE_VALUE) throw Pothos::RuntimeException("Pothos::Util::FileLock("+filePath+")", std::to_string(GetLastError()));
}

Pothos::Util::FileLock::~FileLock(void)
{
    if (_impl->locked) this->unlock();
    CloseHandle(_impl->handle);
    delete _impl;
}

void Pothos::Util::FileLock::lock(void)
{
    if (_impl->locked) return;
    LARGE_INTEGER fileSize;
    GetFileSizeEx(_impl->handle, &fileSize);
    BOOL ret = LockFileEx(
        _impl->handle,
        LOCKFILE_EXCLUSIVE_LOCK,
        0,
        fileSize.LowPart,
        fileSize.HighPart,
        &_impl->overlapped);
    if (ret != TRUE) throw Pothos::RuntimeException("Pothos::Util::FileLock::lock()", std::to_string(GetLastError()));
    _impl->locked = true;
}

void Pothos::Util::FileLock::unlock(void)
{
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
}

