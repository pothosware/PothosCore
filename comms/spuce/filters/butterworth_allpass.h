#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <cmath>
#include <vector>
#include <spuce/typedefs.h>
namespace spuce {
//! \file
//! \brief Calculate Allpass coefficients for halfband bireciprocal wave filter
//
//! assuming butterworth frequency response
//! \ingroup functions iir
//!  \author Tony Kirke,  Copyright(c) 2001
void butterworth_allpass(std::vector<float_type>& a0, std::vector<float_type>& a1, int L);

}  // namespace spuce
