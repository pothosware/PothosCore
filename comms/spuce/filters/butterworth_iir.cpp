// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#include <spuce/filters/butterworth_iir.h>
#include <cfloat>
namespace spuce {
//! Constructor, fcd = cut-off (1=sampling rate)
//! ord = Filter order
//! amax = attenuation at cut-off
void butterworth_iir(iir_coeff& filt, float_type fcd, float_type amax = 3.0) {
  // amax - attenuation at cutoff
  const float_type ten = 10.0;
  auto order = filt.getOrder();
  float_type epi = pow((float_type)(pow(ten, amax / ten) - 1.0), (float_type)(1. / (2.0 * order)));
  // fcd - desired cutoff frequency (normalized)
  float_type wca = (filt.get_type()==filter_type::low) ? tan(M_PI * fcd) / epi : tan(M_PI*(0.5-fcd))/ epi;
  // wca - pre-warped angular frequency
  auto n2 = (order + 1) / 2;
  butterworth_s(filt, wca, order, n2);
  filt.bilinear();
  filt.convert_to_ab();
}
//! Calculate roots
void butterworth_s(iir_coeff& filt, float_type wp, long n, long n2) {
  auto l = (n % 2 == 0) ? 1 : 0;
  float_type arg;
  for (int j = 0; j < n2; j++) {
    arg = -0.5 * M_PI * l / ((float_type)(n));
    if (filt.get_type()==filter_type::low) {
      filt.set_pole(wp * exp(std::complex<double>(0, arg)), j);
      filt.set_zero(FLT_MAX, j);
    } else {
      filt.set_pole((1.0 / wp) * exp(std::complex<double>(0, arg)), j);
      filt.set_zero(0, j);
    }
    l += 2;
  }
}
}  // namespace spuce
