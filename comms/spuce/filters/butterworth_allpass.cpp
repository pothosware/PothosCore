// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <spuce/filters/butterworth_allpass.h>
namespace spuce {
//! creates arrays of coefficents to 2-branch IIR consisting of 1st order allpass filters	for a given order L
//! design criteria is maximally flat in passband	
void butterworth_allpass(std::vector<float_type>& a0, std::vector<float_type>& a1, int L) {
  int N = 2 * L + 1;
  int J = L / 2;
  int l;
  float_type d;
  for (l = 1; l <= J; l++) {
    d = tan(M_PI * l / N);
    a1[l - 1] = d * d;
  }
  for (l = J + 1; l <= L; l++) {
    d = 1.0 / tan(M_PI * l / N);
    a0[l - J - 1] = d * d;
  }
}
}  // namespace spuce
