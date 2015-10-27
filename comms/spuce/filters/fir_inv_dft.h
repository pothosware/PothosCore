#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <vector>
namespace spuce {
//! \file
//! \brief Calculate coefficients for FIR using frequency sampling IDFT
//! \author Tony Kirke
//! \ingroup functions fir
std::vector<float_type> inv_dft_symmetric(const std::vector<float_type>& A, int N);
std::vector<float_type> inv_dft(const std::vector<float_type>& A, int N);
}  // namespace spuce
