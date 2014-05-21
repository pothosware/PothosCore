//
// Util/CompareTo.hpp
//
// Utility functions for comparing objects.
//
// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <typeinfo>
#include <string>

namespace Pothos {
namespace Util {

/*!
 * Compare two objects and return a integer result.
 * The objects being compared require operator==() and operator<().
 * \param v0 value on the left hand side of the comparison
 * \param v1 value on the right hand side of the comparison
 * \return 0 for equal, -1 if v0 < v1, +1 if v0 > v1
 */
template <typename T0, typename T1>
int compareTo(const T0 &v0, const T1 &v1)
{
    if (v0 == v1) return 0;
    if (v0 < v1) return -1;
    else return +1;
}

} //namespace Util
} //namespace Pothos
