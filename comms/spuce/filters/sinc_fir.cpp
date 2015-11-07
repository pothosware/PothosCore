// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <cmath>
#include <spuce/typedefs.h>
#include <spuce/filters/fir_coeff.h>
#include <spuce/filters/sinc_fir.h>
namespace spuce {
void sinc_fir(fir_coeff<float_type>& sincf, float_type f) {
  size_t len = sincf.number_of_taps();
	std::vector<float_type> taps(len);
	float_type sum=0;
	for (size_t i = 0; i < len; i++) {
		double x = i - (len - 1.0) / 2.0;
		if( x == 0.0 ) taps[i] = f;
		else taps[i] = sin( x * f * M_PI) / (x * M_PI);
		sum += taps[i];
	}
	for (size_t i = 0; i <len; i++) sincf.settap(i,taps[i]/sum);
}
}  // namespace spuce
