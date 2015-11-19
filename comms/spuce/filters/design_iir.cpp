// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#define _USE_MATH_DEFINES
#include <spuce/filters/design_iir.h>
#include <spuce/filters/butterworth_iir.h>
#include <spuce/filters/chebyshev_iir.h>
#include <spuce/filters/chebyshev2_iir.h>
#include <spuce/filters/elliptic_iir.h>
#include <iostream>
namespace spuce {
//! \file
//! \brief Design functions for iir filters
//! \author Tony Kirke
//! \ingroup functions iir
iir_coeff* design_iir(const std::string& iir_type, const std::string& filt_type,
                      int order, float_type fcd,
                      float_type ripple, float_type stopattn,
                      float_type center_freq) {
  iir_coeff* filt = new iir_coeff(order);
  if (filt_type == "LOW_PASS") {
    filt->set_type(filter_type::low);
  } else if (filt_type == "HIGH_PASS") {
    filt->set_type(filter_type::high);
  } else if (filt_type == "BAND_PASS") {
    filt->set_type(filter_type::bandpass);
  } else if (filt_type == "BAND_STOP") {
    filt->set_type(filter_type::bandstop);
  } else {
    std::cout << "Unsupported band type :" << filt_type << "\n";
  }

	// Only used by bandpass/bandstop
	filt->set_center(center_freq);
	
  if (iir_type == "butterworth") {
    butterworth_iir(*filt, fcd, 3.0);
  } else if (iir_type == "chebyshev") {
    chebyshev_iir(*filt, fcd, ripple);
  } else if (iir_type == "chebyshev2") {
    chebyshev2_iir(*filt, fcd, stopattn);
  } else if (iir_type == "elliptic") {
    elliptic_iir(*filt, fcd, ripple, stopattn);
  } else {
    std::cout << "Unknown iir type\n";
  }

  return filt;
}
}  // namespace spuce
