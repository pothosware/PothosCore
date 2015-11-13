// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <spuce/filters/elliptic_iir.h>
#include <spuce/filters/elliptic_allpass.h>
#include <vector>
#define MINFP 0.005
namespace spuce {
//! creates arrays of coefficents to 2-branch IIR consisting of 1st order allpass filters	for a given order L
//! design criteria is elliptic shaping for maximal stopband attenuation
//! fp controls the amount of attenuation	
void elliptic_allpass(std::vector<float_type>& a0, std::vector<float_type>& a1, float_type fp, int L) {
  int N = 2 * L + 1;
  int l;
  int i = 0;
  int j = 0;
  int odd;
  float_type lambda = 1.0;
  float_type d, sn, cn, dn, sn2, beta;
  float_type k, k2, zeta, zeta2;

  if (fp < MINFP) fp = MINFP;
  k = 2 * fp;
  zeta = 1.0 / k;
  zeta2 = zeta * zeta;
  ;
  k2 = k * k;

  odd = (L % 2);
  for (l = 1; l <= L; l++) {
    d = ellik(M_PI / 2.0, k);
    d = ((2.0 * l - 1.0) / N + 1.0) * d;
    ellpj(d, k2, sn, cn, dn);
    sn2 = sn * sn;
    beta = (zeta + sn2 - lambda * sqrt((1 - sn2) * (zeta2 - sn2))) /
           (zeta + sn2 + lambda * sqrt((1 - sn2) * (zeta2 - sn2)));
    if (l % 2 != odd)
      a1[i++] = beta;
    else
      a0[j++] = beta;
  }
}
}  // namespace spuce
