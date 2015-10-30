// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#include <cmath>
#include <spuce/typedefs.h>
#include <spuce/filters/fir_coeff.h>
#include <spuce/filters/remez_fir.h>
#include <spuce/filters/create_remez_lpfir.h>
namespace spuce {
//! \brief calculates the coefficients for lowpass FIR based on Remez constraints
void create_remez_lpfir(fir_coeff<float_type>& remezfir, float_type pass_edge, float_type stop_edge,
                        float_type stop_weight) {
  bool ok = true;
  std::vector<float_type> e1(4);
  std::vector<float_type> f1(4);
  std::vector<float_type> w1(4);
  long nfilt = remezfir.number_of_taps();
  remez_fir Remz;
  w1[0] = 1.0;
  w1[1] = stop_weight;
  e1[0] = 0;
  e1[1] = pass_edge / 2.0;
  e1[2] = stop_edge / 2.0;
  e1[3] = 0.5;
  f1[0] = 1.0;
  f1[1] = 0.0;
  std::vector<float_type> fir_coef(nfilt);
  ok = Remz.remez(fir_coef, nfilt, 2, e1, f1, w1, remez_type::BANDPASS);
  if (ok) {
    for (int i = 0; i < nfilt; i++) remezfir.settap(i, fir_coef[i]);
  } else {
    for (int i = 0; i < nfilt; i++) remezfir.settap(i, 0);
    remezfir.settap(0, 1);
  }
}
}  // namespace spuce
