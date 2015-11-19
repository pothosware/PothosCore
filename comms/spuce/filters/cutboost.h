#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/typedefs.h>
#include <spuce/filters/allpass_2nd.h>
namespace spuce {
//! \file
//! \brief IIR structure that can cut or boost signals at a programmed frequency
//! This has independent controls for notch frequency and depth of notch
//! \author Tony Kirke
//! \image html cutboost.png
//! \ingroup double_templates iir
template <class Numeric, class Coeff = float_type> class cutboost {
 public:
  allpass_2nd<Numeric, Coeff> ap;
  float_type k;
  float_type g;
  float_type a;

  cutboost() : ap(0.1, -cos(2.0*M_PI * 0.2) * (1 + 0.1)) {
    a = 0.9;
    k = 1.0;
    g = 0.50;
  }
  cutboost(float_type fp, float_type k2, float_type k0) : ap(k2, -cos(2.0*M_PI * fp) * (1 + k2)) {
    a = k2;
    k = (1 - k0) / (1 + k0);
    g = 0.5 * (1 + k0);
  }
  cutboost(float_type fp, float_type k0) : ap(0.9, -cos(2.0*M_PI * fp) * (1 + 0.9)), k(1) {
    a = 0.9;
    k = (1 - k0) / (1 + k0);
    g = 0.5 * (1 + k0);
  }
  void reset() { ap.reset(); }
  Numeric clock(Numeric x) {
    Numeric sum = (Numeric)(g * (x + k * ap.clock(x)));
    return (sum);
  }
  void set_coeffs(float_type fp, float_type k2, float_type k0) {
    ap.set_coeffs(k2, -cos(2.0*M_PI * fp) * (1 + k2));
    a = k2;
    k = (1 - k0) / (1 + k0);
    g = 0.5 * (1 + k0);
  }
  void set_depth(float_type k0) {
    k = (1 - k0) / (1 + k0);
    g = 0.5 * (1 + k0);
  }
  void set_freq(float_type fp) { ap.set_coeffs(a, -cos(2.0*M_PI * fp) * (1 + a)); }
};
// template_instantiations: float_type
}  // namespace SPUC
