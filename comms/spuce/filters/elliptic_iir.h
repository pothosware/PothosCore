#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/iir_coeff.h>
namespace spuce {
//! \file
//! \brief Calculate coefficients for IIR assuming elliptic frequency response
//
//! \author Tony Kirke,  Copyright(c) 2001
//! \ingroup functions iir
void elliptic_iir(iir_coeff& filt, float_type fcd, float_type fstop, float_type stopattn, float_type ripple);
//! get roots in Lamda plane
float_type lamda_plane(float_type k, float_type m, int n, float_type eps);
//! calculate s plane poles and zeros
void s_plane(iir_coeff& filt, int n, float_type u, float_type m, float_type k, float_type Kk, float_type wc);
float_type ellik(float_type phi, float_type k);
float_type ellpk(float_type k);
int ellpj(float_type u, float_type m, float_type& sn, float_type& cn, float_type& dn);
float_type msqrt(float_type u);
}  // namespace spuce
