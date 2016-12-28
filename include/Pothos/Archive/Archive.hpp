///
/// \file Archive/Archive.hpp
///
/// Templated serialization API.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <typeinfo>
#include <iosfwd>

namespace Pothos {
namespace Archive {

template <typename T>
struct ArchiveEntryContainer;

class POTHOS_API OStreamArchiver
{
public:
    OStreamArchiver(std::ostream &os, const int ver = 0):
        os(os), ver(ver)
    {
        return;
    }

    template <typename T>
    void operator&(const T &value)
    {
        this->writeType(typeid(value));
        ArchiveEntryContainer<T>::save(*this, value, ver);
    }

    template <typename T>
    void operator<<(const T &value)
    {
        *this & value;
    }

    void writeInt32(const int num);
    void writeInt64(const long long num);
    void writeBytes(const void *buff, const size_t len);

private:

    void writeType(const std::type_info &type);

    std::ostream &os;
    const int ver;
};
class POTHOS_API IStreamArchiver
{
public:
    IStreamArchiver(std::istream &is, const int ver = 0):
        is(is), ver(ver)
    {
        return;
    }

    template <typename T>
    void operator&(T &value)
    {
        ArchiveEntryContainer<T>::load(*this, value, ver);
    }

    template <typename T>
    void operator>>(T &value)
    {
        *this & value;
    }

    int readInt32(void);
    long long readInt64(void);
    void readBytes(void *buff, const size_t len);

private:

    std::istream &is;
    const int ver;
};

class POTHOS_API ArchiveEntry
{
public:

    ArchiveEntry(const std::type_info &type, const char *id);
};

} //namespace Archive
} //namespace Pothos

