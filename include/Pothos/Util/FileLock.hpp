///
/// \file Util/FileLock.hpp
///
/// A simple blocking file-lock implementation.
///
/// \copyright
/// Copyright (c) 2015-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <string>
#include <memory>

namespace Pothos {
namespace Util {

/*!
 * FileLock implements a portable file locking for use with std::lock_guard.
 * Poco didn't provide a file lock implementation at the time of writing;
 * this implementation uses flock() on Unix and LockFileEx() on Windows.
 */
class POTHOS_API FileLock
{
public:

    /*!
     * Create a file lock object.
     * This does not acquire the lock.
     */
    FileLock(const std::string &filePath);

    /*!
     * Destroy a file lock object.
     * Release the file lock if acquired.
     */
    ~FileLock(void);

    //! Perform lock operation
    void lock(void);

    //! Perform unlock operation
    void unlock(void);

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;

    FileLock(const FileLock&){}
    void operator=(const FileLock&){}
};

} //namespace Util
} //namespace Pothos
