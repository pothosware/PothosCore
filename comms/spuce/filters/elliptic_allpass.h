#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/typedefs.h>
#include <cmath>
#include <vector>
namespace spuce {
//! \file
//! \brief Calculate Allpass coefficients for halfband bireciprocal wave filter
//! assuming elliptic frequency response
//! \ingroup functions iir
//!  \author Tony Kirke,  Copyright(c) 2015
void elliptic_allpass(std::vector<float_type>& a0, std::vector<float_type>& a1, float_type fp, int L);

}  // namespace spuce
