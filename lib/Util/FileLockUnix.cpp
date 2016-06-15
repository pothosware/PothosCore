// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/FileLock.hpp>
#include <Pothos/Exception.hpp>

#include <sys/file.h> //flock
#include <fcntl.h> //open
#include <unistd.h> //close
#include <cstring> //strerror
#include <cerrno> //errno

#ifdef __FreeBSD__
#include <sys/stat.h>
#endif

struct Pothos::Util::FileLock::Impl
{
    Impl(void):
        fd(-1),
        locked(false)
    {
        return;
    }
    int fd;
    bool locked;
};

Pothos::Util::FileLock::FileLock(const std::string &filePath):
    _impl(new Impl())
{
    _impl->fd = open(filePath.c_str(), O_RDONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (_impl->fd == -1) throw Pothos::RuntimeException("Pothos::Util::FileLock("+filePath+")", strerror(errno));
}

Pothos::Util::FileLock::~FileLock(void)
{
    if (_impl->locked) this->unlock();
    close(_impl->fd);
}

void Pothos::Util::FileLock::lock(void)
{
    if (_impl->locked) return;
    int ret = flock(_impl->fd, LOCK_EX);
    if (ret != 0) throw Pothos::RuntimeException("Pothos::Util::FileLock::lock()", strerror(errno));
    _impl->locked = true;
}

void Pothos::Util::FileLock::unlock(void)
{
    if (not _impl->locked) return;
    int ret = flock(_impl->fd, LOCK_UN);
    if (ret != 0) throw Pothos::RuntimeException("Pothos::Util::FileLock::unlock()", strerror(errno));
    _impl->locked = false;
}
