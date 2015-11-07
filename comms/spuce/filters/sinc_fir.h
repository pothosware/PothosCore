#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/filters/fir_coeff.h>
namespace spuce {
//! \file
//! \brief Calculate coefficients for FIR assuming box-car filter defined by sinc function
//! \ingroup functions fir
void sinc_fir(fir_coeff<float_type>& sinc, float_type f);
}  // namespace spuce
