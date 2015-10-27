// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <spuce/filters/chebyshev2_iir.h>
#include <cfloat>

#include <iostream>
namespace spuce {
//! fcd = cut-off (1=sampling rate)
//! ord = Filter order
//! ripple = passband ripple in dB
void chebyshev2_iir(iir_coeff& filt, float_type fcd, float_type stopband = 40.0) {
  const float_type ten = 10.0;
  auto order = filt.getOrder();
  float_type delta = pow(ten, -stopband/20.0);
  float_type epi   = delta/sqrt(1 - delta*delta);
  float_type wca = (filt.get_type()==filter_type::high) ? tan(M_PI * (0.5-fcd)) : tan(M_PI*fcd);
  chebyshev2_s(filt, wca, epi, order);
	filt.bilinear();
	if (filt.get_type()==filter_type::bandpass || filt.get_type()==filter_type::bandstop) {
		filt.make_band(filt.get_center());
	} else {
		filt.convert_to_ab();
	}
	if (filt.get_type()==filter_type::bandpass) filt.set_bandpass_gain();
}
//! Calculate poles (chebyshev)
void chebyshev2_s(iir_coeff& filt, float_type wp, float_type epi, long order) {
  auto l = 1;
  int n2 = (order + 1) / 2;
  float_type x = 1 / epi;
  float_type lambda = pow(x*(1.0 + sqrt(1.0 + epi*epi)),1.0/order);
  float_type sm = 0.5*((1.0/lambda) - lambda);
  float_type cm = 0.5*((1.0/lambda) + lambda);
  for (int j = 0; j < n2; j++) {
    float_type arg  = M_PI * (2*l-1) / ((float_type)(2*order));
		std::complex<float_type> p = std::complex<float_type>(sm * sin(arg), cm * cos(arg));
    if (filt.get_type()==filter_type::low) {
			// Get regular chebyshev pole first
			// then transform for inverse chebyshev
			filt.set_pole((-wp*p/norm(p)), n2-1-j);
			// inverse chebyshev zero
			filt.set_zero(std::complex<float_type>(0,wp/cos(arg)),n2-1-j);
    } else {
			filt.set_pole(-p/wp, n2-1-j);
			filt.set_zero(std::complex<float_type>(0,cos(arg)/wp),n2-1-j);
    }
    l++;
  }
}
}  // namespace spuce
