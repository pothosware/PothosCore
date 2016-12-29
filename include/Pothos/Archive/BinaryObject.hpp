///
/// \file Archive/BinaryObject.hpp
///
/// String binary chunk serialization.
///
/// \copyright
/// Copyright (c) 2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/Split.hpp>
#include <cstddef> //size_t

namespace Pothos {
namespace serialization {

/*!
 * Wrap a chunk given a pointer and length in bytes.
 * This interface is lightweight and does not copy.
 */
class BinaryObject
{
public:
    BinaryObject(const void * const buff, const size_t len):
        buff(buff), len(len)
    {
        return;
    }

    template <typename Archive>
    void save(Archive &ar, const unsigned int)
    {
        ar.writeBytes(buff, len);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int)
    {
        ar.readBytes(const_cast<void *>(buff), len);
    }

    POTHOS_SERIALIZATION_SPLIT_MEMBER()

private:
    const void * const buff;
    const size_t len;
};

}}
