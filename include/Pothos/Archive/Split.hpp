///
/// \file Archive/Split.hpp
///
/// Serialization dispatch macros for separate save/load.
///
/// \copyright
/// Copyright (c) 2016-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Archive/Invoke.hpp>
#include <type_traits>

/*!
 * Declare a serialize() function that can dispatch to
 * an individually declared save and load function.
 * Call in the outside scope, no namespaces or functions.
 */
#define POTHOS_SERIALIZATION_SPLIT_FREE(T) \
    namespace Pothos { namespace serialization { \
        template <typename Archive> \
        void serialize(Archive &ar, T &t, const unsigned int ver) { \
            Pothos::serialization::invokeSplit(ar, t, ver); \
        } \
    }}

/*!
 * Declare a serialize() function that can dispatch to
 * individually declared save and load member functions.
 * Call in the public part of a struct or class declaration.
 */
#define POTHOS_SERIALIZATION_SPLIT_MEMBER() \
    template <typename Archive> \
    typename std::enable_if<Archive::isSave::value>::type \
    serialize(Archive &ar, const unsigned int ver) { \
        this->save(ar, ver); \
    } \
    template <typename Archive> \
    typename std::enable_if<!Archive::isSave::value>::type \
    serialize(Archive &ar, const unsigned int ver) { \
        this->load(ar, ver); \
    }
