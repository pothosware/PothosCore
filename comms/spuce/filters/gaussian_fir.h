#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/filters/fir_coeff.h>
namespace spuce {
//! \file
//! \brief Calculate coefficients for FIR assuming gaussian frequency response
//! \author Tony Kirke,  Copyright(c) 2001
//! \ingroup functions fir
void gaussian_fir(fir_coeff<float_type>& gaussf, float_type bt, float_type spb);
}  // namespace spuce
