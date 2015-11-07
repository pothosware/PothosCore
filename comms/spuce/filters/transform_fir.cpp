#define _USE_MATH_DEFINES
#include <cmath>
#include <spuce/filters/transform_fir.h>
#include <iostream>
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
namespace spuce {
//! \author Tony Kirke
//! \ingroup functions fir
std::vector<float_type> transform_fir(const std::string& band_type, const std::vector<float_type>& taps, float_type f) {
	std::vector<float_type> out(taps.size());
  if (band_type == "LOW_PASS") {
		// just copy
		for (size_t i=0;i<taps.size();i++) {
			out[i] = taps[i];
		}
  } else if (band_type == "HIGH_PASS") {
		float_type h = 1;
		// Alternate multiplying by +1,-1
		for (size_t i=0;i<taps.size();i++) {
			out[i] = h*taps[i];
			h *= -1;
		}
  } else if (band_type == "BAND_PASS") {
		// Modulate taps up to band center frequency
		for (size_t i=0;i<taps.size();i++) {
			// Scale to restore center tap value & by 0.5 due to cosine multiplication
			out[i] = cos(2.0*M_PI*f*i) * taps[i] / (0.5*cos(M_PI*f*(taps.size()-1)));
		}
  } else if (band_type == "BAND_STOP") {
		// First transform to band_pass, then subtract 1 from center tap
		out = transform_fir("BAND_PASS", taps, f);
		int mid_tap = (taps.size()-1)/2;
		out[mid_tap] -= 1;
  } else {
    std::cout << "Unsupported band type :" << band_type << "\n";
  }
	return out;
}
std::vector<std::complex<float_type>> transform_complex_fir(const std::string& band_type, const std::vector<float_type>& taps, float_type f) {
	std::vector<std::complex<float_type>> out(taps.size());
  if (band_type == "COMPLEX_BAND_PASS") {
		// Modulate taps up to band center frequency
		for (size_t i=0;i<taps.size();i++) {
			double phase = -2.0*M_PI*f*i;
			out[i] = taps[i]*std::polar(1.0, phase);
		}
  } else if (band_type == "COMPLEX_BAND_STOP") {
		out = transform_complex_fir("COMPLEX_BAND_PASS", taps, f);
		int mid_tap = (taps.size()-1)/2;
		out[mid_tap] -= 1;
  } else {
    std::cout << "Unsupported band type :" << band_type << "\n";
  }
	return out;
}
}  // namespace spuce
