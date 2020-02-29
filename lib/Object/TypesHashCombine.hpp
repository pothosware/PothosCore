// Copyright (c) 2020-2020 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <typeinfo>
#include <cstddef>

//! combine two type hashes to form a unique hash such that hash(a, b) != hash(b, a)
static inline size_t typesHashCombine(const size_t &inTypeHash, const size_t &outTypeHash) noexcept
{
    return inTypeHash + 0x9e3779b9 + (outTypeHash<<6) + (outTypeHash>>2);
}

static inline size_t typesHashCombine(const std::type_info &inType, const std::type_info &outType) noexcept
{
    return typesHashCombine(inType.hash_code(), outType.hash_code());
}
