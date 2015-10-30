// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
// from directory: src
#include <cmath>
#include <spuce/typedefs.h>
#include <spuce/complex.h>
#include <spuce/filters/fir_coeff.h>
#include <spuce/filters/remez_fir.h>
namespace spuce {
void create_remez_fir(fir_coeff<float_type>& remezfir, int jtype, int nbands, std::vector<float_type>& edge,
                      std::vector<float_type>& fx, std::vector<float_type>& wtx) {
  bool ok;
  long nfilt = remezfir.number_of_taps();
  remez_fir Remz;
  std::vector<float_type> fir_coef(nfilt);
  ok = Remz.remez(fir_coef, nfilt, nbands, edge, fx, wtx, jtype);
  if (!ok) {
    for (int i = 0; i < nfilt; i++) remezfir.settap(i, 0);
    remezfir.settap(0, 1);
  } else {
    for (int i = 0; i < nfilt; i++) remezfir.settap(i, fir_coef[i]);
  }
}
}  // namespace spuce
