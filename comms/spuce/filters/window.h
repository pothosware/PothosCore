#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <vector>
namespace spuce {
//! \file
//! \brief Various FIR window functions: hamming,hanning,blackman,kaiser, chebyshev
//
//! \brief bessel function for kaiser window
//! \author Tony Kirke
//! \ingroup functions fir
float_type io(float_type x);
//! \brief hamming window \f$ w(n) = alpha + beta*cos( 2*\pi*(n-1)/(nf-1) )\f$
//! \author Tony Kirke
//!  \ingroup functions fir
std::vector<float_type> hamming(long nf, float_type alpha, float_type beta);
//! \brief hanning window \f$ w(n) = 0.5( 1 - cos( 2*\pi*n/(nf-1) )\f$
//! \author Tony Kirke
//!  \ingroup functions fir
std::vector<float_type> hanning(long nf);
//! \brief Blackman Window	\f$ w[x] = 0.42 - 0.5*cos(2*\pi*x/nf) + 0.08*cos(2*\pi*x/nf)\f$
//! \author Tony Kirke
//!  \ingroup functions fir
std::vector<float_type> blackman(long nf);
//! \brief kaiser window
//! \author Tony Kirke
//!  \ingroup functions fir
std::vector<float_type> kaiser(long nf, float_type tw, float_type ripple);
//!  \brief dolph chebyshev window design
//! \author Tony Kirke
//!  \ingroup functions fir
std::vector<float_type> cheby(long nf, long n, long ieo, float_type dp, float_type df, float_type x0);
//! \brief chebyshev window
//
//! subroutine to generate chebyshev window parameters when
//! one of the three parameters nf,dp and df is unspecified
//! \author Tony Kirke
//!  \ingroup functions fir
void chebc(float_type nf, float_type dp, float_type df, float_type n, float_type x0);
	//! \brief bartlett window design
std::vector<float_type> bartlett(long nf);
}  // namespace spuce
