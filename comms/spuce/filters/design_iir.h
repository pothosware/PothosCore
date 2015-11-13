#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <string>
#include <spuce/typedefs.h>
#include <spuce/filters/iir_coeff.h>
namespace spuce {
//! \file
//! \brief Design functions for iir filters
//! \author Tony Kirke
//! \ingroup functions iir
iir_coeff* design_iir(const std::string& iir_type, const std::string& filt_type,
                      int order, float_type fcd,
                      float_type ripple = 0.1, float_type stopattn = 60,
                      float_type center_freq = 0.25);
}  // namespace spuce
