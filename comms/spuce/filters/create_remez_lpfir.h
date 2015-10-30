#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/filters/fir_coeff.h>
namespace spuce {
//! \file
//! \brief Calculate coefficients for lowpass FIR assuming equiripple frequency response
//! \author Tony Kirke,  Copyright(c) 2001
//! \ingroup functions fir
void create_remez_lpfir(fir_coeff<float_type>& remezfir, float_type edge, float_type fx, float_type wtx);
}  // namespace spuce
