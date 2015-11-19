// Copyright (c) 2015 Tony Kirke
// SPDX-License-Identifier: BSL-1.0
#define _USE_MATH_DEFINES
#include <algorithm>
#include <spuce/filters/design_fir.h>
#include <spuce/filters/butterworth_fir.h>
#include <spuce/filters/sinc_fir.h>
#include <spuce/filters/gaussian_fir.h>
#include <spuce/filters/remez_fir.h>
#include <spuce/filters/raised_cosine.h>
#include <spuce/filters/root_raised_cosine.h>
#include <spuce/filters/transform_fir.h>
#include <spuce/filters/sinc_helper.h>
namespace spuce {
//! \file
//! \brief Design functions for fir filters
//! \author Tony Kirke
//! \ingroup functions fir
std::vector<double> design_fir(const std::string& fir_type,
                               const std::string& band_type,                               
															 int order, float_type fl, float_type fu,
                               float_type alpha_beta_stop_edge,
                               float_type weight) {

  std::vector<float_type> taps;
  fir_coeff<float_type> filt;
  filt.set_size(order);

  float_type filt_bw;
  float_type irate;
  float_type center_frequency = 0.5*(fu+fl);

  // Handle Remez separately
  if (fir_type == "remez") {
    std::vector<float_type> bands(4);
    std::vector<float_type> des(4);
    std::vector<float_type> w(4);
    taps.resize(order);
    remez_fir Remz;

    if ((band_type == "BAND_PASS") || (band_type == "BAND_STOP")) {
      filt_bw = 0.5*(fu - fl);
    } else {
      // For Remez keep bw = fl
      filt_bw = fl;
    }
    float_type stop_freq = alpha_beta_stop_edge + filt_bw;
    bands[0] = 0;
    bands[1] = filt_bw;
    bands[2] = stop_freq;
    bands[3] = 0.5;
    if (band_type == "HIGH_PASS") {
      des[0] = 0.0;
      des[1] = 1.0;
      w[0] = weight;
      w[1] = 1.0;
    } else {
      des[0] = 1.0;
      des[1] = 0.0;
      if (band_type == "BAND_STOP") { 
        w[0] = weight;
        w[1] = 1.0;
      } else {
        w[0] = 1.0;
        w[1] = weight;
      }
    }
    //std::cout << "Remez : " << bands[1] << " " << bands[2] << " ";
    //std::cout << " des : " << des[0] << " " << des[1] << " ";
    //std::cout << " weight : " << w[0] << " " << w[1] << "\n";
    Remz.remez(taps, order, 2, bands, des, w, remez_type::BANDPASS);

    if ((band_type == "BAND_PASS") || (band_type == "BAND_STOP")) {
      taps = transform_fir(band_type, taps, center_frequency);
    }
  } else {
    irate   = 1.0/fl;
    if ((band_type == "BAND_PASS") ||
        (band_type == "BAND_STOP")) {
      // Since bandpass/stop, 1/2 the band-pass filter bandwidth since double sided
      // Also bandwidth is absolute value
      filt_bw = std::abs(0.5*(fu-fl));
      irate   = 1.0/std::abs(0.5*(fu-fl));
    } else if (band_type == "HIGH_PASS") {
      filt_bw = 0.5 - fl;
    } else {
      filt_bw = fl;
    }

    if (fir_type == "maxflat") {
      filt_bw = std::max(filt_bw,0.02);
      butterworth_fir(filt, filt_bw);
      taps = get_taps(filt);
    } else if (fir_type == "gaussian") {
      if (band_type == "HIGH_PASS") filt_bw = fl;
      gaussian_fir(filt,filt_bw);
      taps = get_taps(filt);
    } else if (fir_type == "raised_cosine") {
      raised_cosine(filt, alpha_beta_stop_edge, irate);
      taps = get_taps(filt);
    } else if (fir_type == "root_raised_cosine") {
      root_raised_cosine(filt, alpha_beta_stop_edge, irate);
      taps = get_taps(filt);
    } else if (fir_type == "sinc") {
      sinc_fir(filt,filt_bw);
      taps = get_taps(filt);
    } else {
      std::string err = "Unknown fir type : " + fir_type;
      throw std::runtime_error(err);
    }

    if (taps.size() > 0) {
      // Transform Taps!!
      if (band_type == "HIGH_PASS") {
        // Treat these filters differently for high pass since not quite possible to use high-pass transform
        if ((fir_type == "raised_cosine") || (fir_type == "root_raised_cosine")
            || (fir_type == "gaussian"))  {
          taps = transform_fir("INVERSE", taps, center_frequency);
        } else {
          taps = transform_fir("HIGH_PASS", taps, center_frequency);
        }
      } else if (band_type == "BAND_PASS") {
        taps = transform_fir("BAND_PASS", taps, center_frequency);
      } else if (band_type == "BAND_STOP") {
        // Handle Sinc with special function
        if (fir_type == "sinc") {
          taps = sincBSF(order, fl, fu);
        } else if (fir_type == "maxflat") {
          std::string err = "maxflat FIR as BAND_STOP not supported\n";
          throw std::runtime_error(err);
        } else {
          taps = transform_fir("BAND_STOP", taps, center_frequency);
        }
      }
    }
  }
  return taps;
}

std::vector<std::complex<double> > design_complex_fir(const std::string& fir_type,
                                                      const std::string& band_type,                               
                                                      int order, float_type fl, float_type fu,
                                                      float_type alpha_beta_stop_edge,
                                                      float_type weight) {
  
  std::vector<float_type> taps;
  float_type center_frequency = 0.5*(fu+fl);
  // Since bandpass/stop, 1/2 the band-pass filter bandwidth since double sided
  // Also bandwidth is absolute value
  float_type filt_bw = std::abs(0.5*(fu-fl));

  if ((fir_type == "sinc") && (band_type == "COMPLEX_BAND_STOP")) {
    taps = design_fir(fir_type, "HIGH_PASS", order, filt_bw, fu, alpha_beta_stop_edge, weight);
    auto complex_taps = transform_complex_fir("COMPLEX_BAND_PASS", taps, center_frequency);
    return complex_taps;
  } else {
    taps = design_fir(fir_type, "LOW_PASS", order, filt_bw, fu, alpha_beta_stop_edge, weight);
    auto complex_taps = transform_complex_fir(band_type, taps, center_frequency);
    if ((fir_type == "maxflat") && (band_type == "COMPLEX_BAND_STOP")) {
      std::string err = "maxflat FIR as COMPLEX_BAND_STOP not supported\n";
      throw std::runtime_error(err);
    }
    return complex_taps;
  }
}
}  // namespace spuce
