#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <vector>
#include <spuce/typedefs.h>
namespace spuce {
//! \file
//! \brief Various FIR window functions: hamming,hanning,blackman,kaiser, chebyshev
//
//! \brief rectangular window \f$ w(n) = 1;\f$
//!  \ingroup functions fir
std::vector<float_type> rectangular(long nf);
//!  \ingroup functions fir
std::vector<float_type> flattop(long nf);
//! \brief hamming window \f$ w(n) = 0.54 - 0.46*cos( 2*\pi*n/(nf-1) )\f$
//!  \ingroup functions fir
std::vector<float_type> hamming(long nf);
//! \brief hanning window \f$ w(n) = 0.5( 1 - cos( 2*\pi*n/(nf-1) )\f$
//!  \ingroup functions fir
std::vector<float_type> hann(long nf);
//!  \ingroup functions fir
std::vector<float_type> hanning(long nf);
//! \brief Blackman Window	\f$ w[x] = 0.42 - 0.5*cos(2*\pi*x/nf) + 0.08*cos(2*\pi*x/nf)\f$
//!  \ingroup functions fir
std::vector<float_type> blackman(long nf);
//! \brief kaiser window
//!  \ingroup functions fir
std::vector<float_type> kaiser(long nf, float_type beta);
//!  \brief dolph chebyshev window design
//!  \ingroup functions fir
std::vector<float_type> cheby(int nf, float_type alpha);
//! \brief bartlett window design
//!  \ingroup functions fir
std::vector<float_type> bartlett(long nf);
//! \brief calculate beta for kaiser window for a given ripple
float_type kaiser_beta(float_type beta);
//! \brief bessel function for kaiser window
float_type io(float_type x);

}  // namespace spuce
