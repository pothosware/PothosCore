// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <cmath>
#include <spuce/typedefs.h>
#include <spuce/filters/fir_coeff.h>
#include <spuce/filters/gaussian_fir.h>
namespace spuce {
//! \file
//! \brief calculates the sampled Gaussian filter impulse response
void gaussian_fir(fir_coeff<float_type>& gaussf, float_type bt, float_type spb) {
  // generate the sampled Gaussian filter impulse response
  // and transfer coefficients to "gaussf".
  // spb   = samples per bit (at full rate)
  // BT    = filter bandwidth
  int i;
  float_type s, t0, ts, gsum;
  int nir = gaussf.number_of_taps();
  float_type* gf = new float_type[nir];
  // dt = 1.0/spb;
  s = 1.0 / (sqrt(log(2.0)) / (2.0 * M_PI * bt));
  t0 = -0.5 * nir;
  gsum = 0.0;  // std::normalize to unit sum
  for (i = 0; i < nir; i++) {
    t0++;
    ts = s * t0;
    gf[i] = ::exp(-0.5 * ts * ts);
    gsum += gf[i];
  }
  // Unity DC gain
  gsum = 1.0 / gsum;
  for (i = 0; i < nir; i++) gaussf.settap(i,gf[i] * gsum);
}
}  // namespace spuce
