///
/// \file Archive/StreamArchiver.hpp
///
/// Archive implementation on top of streaming interfaces.
///
/// \copyright
/// Copyright (c) 2016-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <type_traits>
#include <iosfwd>
#include <cstddef> //size_t

namespace Pothos {
namespace Archive {

/*!
 * The output stream archiver serializes types to an output stream.
 */
class POTHOS_API OStreamArchiver
{
public:
    /*!
     * Create an output stream archiver
     * \param os the stream to write to
     */
    OStreamArchiver(std::ostream &os);

    //! Tell the invoker that this archiver saves
    typedef std::true_type isSave;

    /*!
     * Serialize value with the & operator
     * \param value the value to serialize
     */
    template <typename T>
    void operator&(const T &value);

    /*!
     * Serialize value with the stream operator
     * \param value the value to serialize
     */
    template <typename T>
    void operator<<(const T &value);

    /*!
     * Directly write an array of bytes to the output stream
     */
    void writeBytes(const void *buff, const size_t len);

private:

    std::ostream &os;
    unsigned int ver;
};

/*!
 * The input stream archiver deserializes types from an input stream.
 */
class POTHOS_API IStreamArchiver
{
public:
    /*!
     * Create an input stream archiver
     * \param is the stream to read from
     */
    IStreamArchiver(std::istream &is);

    //! Tell the invoker that this archiver loads
    typedef std::false_type isSave;

    /*!
     * Deserialize value with the & operator
     * \param value the value to deserialize
     */
    template <typename T>
    void operator&(T &value);

    /*!
     * Deserialize value with the stream operator
     * \param value the value to deserialize
     */
    template <typename T>
    void operator>>(T &value);

    /*!
     * Directly read an array of bytes from the input stream
     */
    void readBytes(void *buff, const size_t len);

private:

    std::istream &is;
    unsigned int ver;
};

} //namespace Archive
} //namespace Pothos

//! \cond

#include <Pothos/Archive/Invoke.hpp>

template <typename T>
void Pothos::Archive::OStreamArchiver::operator&(const T &value)
{
    Pothos::serialization::invokeSerialize(*this, const_cast<T &>(value), ver);
}

template <typename T>
void Pothos::Archive::OStreamArchiver::operator<<(const T &value)
{
    *this & value;
}

template <typename T>
void Pothos::Archive::IStreamArchiver::operator&(T &value)
{
    Pothos::serialization::invokeSerialize(*this, value, ver);
}

template <typename T>
void Pothos::Archive::IStreamArchiver::operator>>(T &value)
{
    *this & value;
}

//! \endcond
