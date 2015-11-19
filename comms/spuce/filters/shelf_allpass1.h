#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/allpass_1.h>
namespace spuce {

double z_freqz_db(bool hpf, double f, double a, double h, double l);
double find_a(bool hpf, double f, double des_gain, double h, double l);

//! \file
//! \brief IIR zolzer filter based on 2nd order allpass structure
//! \author Tony Kirke
//! \image html zolzer_allpass.png
//! \author Tony Kirke
//! \ingroup double_templates double_templates iir
template <class Numeric, class Coeff = float_type> class shelf_allpass1 {
 public:
  allpass_1<Numeric, Coeff> ap;
  Coeff low_gain;
  Coeff high_gain;
  bool high_boost;  // high-pass sum case

  // zolzer_allpass(float_type a, float_type g) : ap(a) { gain=g; }
  shelf_allpass1() : ap(0) {}

  void set_coeffs(float_type fc, float_type low_g_db, float_type high_g_db) {
    high_boost = (low_g_db < 0);
    // Convert from dB to gain values
    low_gain = pow(10.0, low_g_db / 20.0);
    high_gain = pow(10.0, high_g_db / 20.0);
    if (high_boost) {
      high_gain = (high_gain - low_gain) / 2.0;
    } else {
      low_gain = (low_gain - high_gain) / 2.0;
    }
    double a = find_a(high_boost, fc, 0.0, high_gain, low_gain);
    ap.init(a, 1);
    reset();
  }

  void reset() { ap.reset(); }

  Numeric clock(Numeric x) {
    Numeric sum;
    if (high_boost) {
      sum = low_gain * x + high_gain * (x - ap.clock(x));
    } else {
      sum = high_gain * x + low_gain * (x + ap.clock(x));
    }
    return (sum);
  }
};

}  // namespace spuce
