#define _USE_MATH_DEFINES
#include <cmath>
#include <stdexcept>
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
      // Scale up by 2.0 due to cosine multiplication
      out[i] = 2.0* cos(2.0*M_PI*f*(i-(taps.size()-1)/2.0)) * taps[i];
    }
  } else if (band_type == "BAND_STOP") {
    // First transform to band_pass, then negate and add 1 at center tap
    out = transform_fir("BAND_PASS", taps, f);
    for (size_t i=0;i<taps.size();i++) out[i] *= -1;
    size_t mid_tap = (taps.size()-1)/2;
    out[mid_tap] += 1;
  } else if (band_type == "INVERSE") {
    for (size_t i=0;i<taps.size();i++) out[i] = -taps[i];
    size_t mid_tap = (taps.size()-1)/2;
    out[mid_tap] += 1;
  } else {
    std::string err = "Unsupported band type in transform_fir :"+band_type;
    throw std::runtime_error(err);
  }
  return out;
}
std::vector<std::complex<float_type>> transform_complex_fir(const std::string& band_type, const std::vector<float_type>& taps, float_type f) {
  std::vector<std::complex<float_type>> out(taps.size());
  if (band_type == "COMPLEX_BAND_PASS") {
    // Modulate taps up to band center frequency
    for (size_t i=0;i<taps.size();i++) {
      double phase = 2.0*M_PI*f*(i - (taps.size()-1)/2.0);
      out[i] = taps[i]*std::polar(1.0, phase);
    }
  } else if (band_type == "COMPLEX_BAND_STOP") {
    out = transform_complex_fir("COMPLEX_BAND_PASS", taps, f);
    for (size_t i=0;i<taps.size();i++) out[i] *= -1;
    size_t mid_tap = (taps.size()-1)/2;
    out[mid_tap] += 1;
  } else {
    std::string err = "Unsupported band type in transform_complex_fir :"+band_type;
    throw std::runtime_error(err);
  }
  return out;
}
}  // namespace spuce
