#pragma once
#include <vector>
#include <spuce/typedefs.h>
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
namespace spuce {
//! \author Tony Kirke
//! \ingroup functions fir
	std::vector<float_type> transform_fir(const std::string& band_type, const std::vector<float_type>& taps, float_type f);
	std::vector<std::complex<float_type> > transform_complex_fir(const std::string& band_type, const std::vector<float_type>& taps, float_type f);
}  // namespace spuce
