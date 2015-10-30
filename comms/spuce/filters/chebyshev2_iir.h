#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/filters/iir_coeff.h>
namespace spuce {
//! \file
//! \brief Calculate coefficients for IIR assuming chebyshev frequency response
//
//! \author Tony Kirke,  Copyright(c) 2015
//! \ingroup functions iir
void chebyshev2_iir(iir_coeff& cheb, float_type fcd, float_type atten);
void chebyshev2_s(iir_coeff& filt, float_type wp, float_type epi, long n);
}  // namespace spuce
