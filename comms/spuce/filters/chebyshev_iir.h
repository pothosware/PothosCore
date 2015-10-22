#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/iir_coeff.h>
namespace spuce {
//! \file
//! \brief Calculate coefficients for IIR assuming chebyshev frequency response
//
//! \author Tony Kirke,  Copyright(c) 2015
//! \ingroup functions iir
void chebyshev_iir(iir_coeff& cheb, float_type fcd, float_type ripple);
void chebyshev_s(iir_coeff& filt, float_type wp, float_type epi, long n, long n2);
}  // namespace spuce
