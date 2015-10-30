#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/filters/fir_coeff.h>
#include <spuce/filters/root_raised_cosine_imp.h>
namespace spuce {
//! \file
//! \brief Root Raised Cosine functions
//! \author Tony Kirke
//! \ingroup template_functions comm
template <class T>
void root_raised_cosine_quantized(fir_coeff<T>& rcfir, float_type alpha, int rate, int bits, float_type scale);
template <class T> void root_raised_cosine(fir_coeff<T>& rcfir, float_type alpha, int rate) {
  int i;
  int num_taps = rcfir.number_of_taps();
  double gain = 1.0 / rate;
  for (i = 0; i < num_taps; i++) {
    rcfir.settap(i,(T)(gain * root_raised_cosine_imp(alpha, float_type(i), (float_type)rate, num_taps)));
  }
}
}  // namespace spuce
