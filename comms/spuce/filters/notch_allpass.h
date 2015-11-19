#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/allpass_2nd.h>
namespace spuce {
//! \file
//! \brief IIR notch filter based on 2nd order allpass structure
//! \author Tony Kirke
//! \ingroup double_templates iir
template <class Numeric, class Coeff = float_type> class notch_allpass {
 public:
  allpass_2nd<Numeric, Coeff> ap;

  notch_allpass(float_type fp, float_type k2) : ap(k2, -cos(2.0*M_PI * fp) * (1 + k2)) {}
  notch_allpass(float_type fp) : ap(0.9, -cos(2.0*M_PI * fp) * (1 + 0.9)) {}
  notch_allpass() : ap(0.9, -cos(2.0*M_PI * 0.2) * (1 + 0.9)) {}
  void set_coeffs(float_type fp, float_type k2) { ap.set_coeffs(k2, -cos(2.0*M_PI * fp) * (1 + k2)); }

  void reset() { ap.reset(); }
  Numeric clock(Numeric x) {
    Numeric sum = 0.5 * (x + ap.clock(x));
    return (sum);
  }
};
}  // namespace spuce
