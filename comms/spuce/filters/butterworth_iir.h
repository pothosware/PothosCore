#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/iir_coeff.h>
namespace spuce {
//! \file
//! \brief Calculate coefficients for IIR assuming butterworth frequency response
//
//! \author Tony Kirke,  Copyright(c) 2001
//! \ingroup functions iir
void butterworth_iir(iir_coeff& filt, float_type fcd, float_type amax);
void butterworth_s(iir_coeff& filt, float_type wp, long n, long n2);
}  // namespace spuce
