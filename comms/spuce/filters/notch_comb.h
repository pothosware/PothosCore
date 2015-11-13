#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/allpass_1.h>
namespace spuce {
//! \file
//! \author Tony Kirke
//! \ingroup double_templates double_templates iir
template <class Numeric, class Coeff = float_type> class notch_comb {
 public:
  allpass_1<Numeric, Coeff> ap;

  notch_comb(float_type fp, int dly) : ap(fp, dly) {}
  notch_comb(float_type fp) : ap(fp, 1) {}
  notch_comb() : ap(0.5, 1) {}
  void set_coeffs(float_type fp, int dly) { ap.init(fp, dly); }

  void reset() { ap.reset(); }
  Numeric clock(Numeric x) {
    Numeric sum = 0.5 * (x + ap.clock(x));
    return (sum);
  }
};

}  // namespace spuce
