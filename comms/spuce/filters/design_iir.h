#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/butterworth_iir.h>
#include <spuce/filters/chebyshev_iir.h>
#include <spuce/filters/elliptic_iir.h>
namespace spuce {
//! \file
//! \brief Design functions for iir filters
//! \author Tony Kirke
//! \ingroup functions iir
iir_coeff* design_iir(const std::string& iir_type, const std::string& filt_type,
											int order, float_type fcd,
                      float_type ripple = 0.1, float_type stopattn = 60,
											float_type bw = 0.1) {
  iir_coeff* filt = new iir_coeff(order);
  if (filt_type == "LOW_PASS") {
    filt->set_type(filter_type::low);
  } else if (filt_type == "HIGH_PASS") {
    filt->set_type(filter_type::high);
  } else {
    std::cout << "Unsupported band type :" << filt_type << "\n";
  }

  if (iir_type == "butterworth") {
    butterworth_iir(*filt, fcd, 3.0);
  } else if (iir_type == "chebyshev") {
    chebyshev_iir(*filt, fcd, ripple);
  } else if (iir_type == "elliptic") {
    elliptic_iir(*filt, fcd, ripple, stopattn, bw);
  } else {
    std::cout << "Unknown iir type\n";
  }

  return filt;
}
}  // namespace spuce
