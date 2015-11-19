#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <string>
#include <vector>
#include <spuce/typedefs.h>
namespace spuce {
//! \file
//! \brief Design functions for fir filters
//! \author Tony Kirke
//! \ingroup functions fir
std::vector<double> design_fir(const std::string& fir_type,
                               const std::string& band_type,
															 int order, float_type fl, float_type fu,
                               float_type alpha_beta_stop_edge=0.1,
                               float_type weight=100.0);

std::vector<std::complex<double> > design_complex_fir(const std::string& fir_type,
                                                      const std::string& band_type,                               
                                                      int order, float_type fl, float_type fu,
                                                      float_type alpha_beta_stop_edge=0.1,
                                                      float_type weight=100.0);

}  // namespace spuce
