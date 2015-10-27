// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
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
  float_type wca = (filt.get_type()==filter_type::high) ? tan(M_PI * (0.5-fcd)) / epi : tan(M_PI*fcd)/ epi;
  // wca - pre-warped angular frequency
  butterworth_s(filt, wca, order);
  filt.bilinear();
	if (filt.get_type()==filter_type::bandpass || filt.get_type()==filter_type::bandstop) {
		filt.make_band(filt.get_center());
	} else {
		filt.convert_to_ab();
	}
	if (filt.get_type()==filter_type::bandpass) filt.set_bandpass_gain();
}
//! Calculate roots
void butterworth_s(iir_coeff& filt, float_type wp, long n) {
  auto l = (n % 2 == 0) ? 1 : 0;
  int n2 = (n + 1) / 2;
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
