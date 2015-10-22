#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <cmath>
#include <vector>
#include <spuce/typedefs.h>
namespace spuce {
//! \file
//! \brief Calculate the std::complex roots of a polynomial equation
//
//! \author Tony Kirke
//! \ingroup functions misc
std::vector<std::complex<float_type> > find_roots(const std::vector<float_type>& a, long n);
}  // namespace spuce
