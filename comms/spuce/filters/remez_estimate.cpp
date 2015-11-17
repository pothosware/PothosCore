// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#include <spuce/filters/remez_estimate.h>
#include <algorithm>
#include <vector>
#include <iostream>
namespace spuce {

float_type remez_estimate_weight(float_type pass_ripple_db, float_type stop_atten_db) {
  // Passband ripple is relative to 1
  double pass_ripple = pow(10.0,pass_ripple_db/20.0) - 1.0;
  // Stopband ripple is -attenation 
  double stop_ripple = pow(10.0,-stop_atten_db/20.0);
  double max_dev = std::max(stop_ripple,pass_ripple);
  double weight;
  if (pass_ripple > stop_ripple) {
    weight = max_dev/stop_ripple;
  } else {
    weight = 1.0/(max_dev/pass_ripple);
  }
  return weight;
}
// Iterate through remez_estimate_num_taps to estimate that for given input parameters, 
// how much small the transition bandwidth can be (approximately)
float_type remez_estimate_bw(size_t num_taps, float_type ripple, float_type atten) {
  size_t num=1;
  float_type trans_bw = 0.45; // should be < 0.5
  do {
    num = remez_estimate_num_taps(trans_bw, ripple, atten);
    trans_bw *= 0.9;
  } while (num < num_taps);
  trans_bw /= 0.9;
  return trans_bw;
}
// Iterate through remez_estimate_num_taps to estimate that for given input parameters, 
// how much stop band attenuation we can achieve (approximately)
float_type remez_estimate_atten(size_t num_taps, float_type trans_bw, float_type ripple) {
  size_t num=1;
  float_type atten = 1.0;
  do {
    num = remez_estimate_num_taps(trans_bw, ripple, atten);
    atten += 1.0;
  } while (num < num_taps);
  atten += 1.0;
  return atten;
}
  
/* Estimate the number of taps for a Remez FIR for the input params */
/*
 *
 * INPUT:
 * -------
 * float_type trans_bw            - transition bandwidth (normalized to sampling rate)
 * float_type ripple              - passband ripple (dB)
 * float_type atten               - stopband attenuation (dB)
 *
 * OUTPUT:
 * -------
 * size_t                         - number of taps required
 *
*/
size_t remez_estimate_num_taps(float_type trans_bw, float_type ripple, float_type atten) {  
  size_t num_taps;
  std::vector<float_type> AA = {-4.278e-01,-4.761e-01,0,
                                -5.941e-01,7.114e-02,0,
                                -2.660e-03,5.309e-03,0};

  // convert to log10
  float_type rl = pow(10.0, ripple/20.0) - 1;
  float_type d1 = log(rl)/log(10.0);
  float_type d2 = -atten/20.0;

  std::vector<float_type> AD(3);
  std::vector<float_type> dd = {1, d2, d2*d2};

  size_t off=0;
  for (size_t j=0;j<3;j++) {
    float_type sum=0;
    for (size_t i=0;i<3;i++) {
      sum += AA[i+off]*dd[i];
    }
    off += 3;
    AD[j] = sum;
  }

  float_type D = AD[0] + d1*AD[1] + d1*d1*AD[2];

  float_type fk = 11.01217 + 0.51244*(d1-d2);
  num_taps = (size_t)ceil(D/trans_bw - fk*trans_bw + 1.0);
  return num_taps;
}
/* Estimate the number of taps for a Remez FIR for the input params */
/*
 *
 * INPUT:
 * -------
 * float_type edge1               - passband edge
 * float_type ripple1             - passband ripple (dB)
 * float_type edge2               - stopband edge
 * float_type ripple1             - stopband ripple (dB)
 *
 * OUTPUT:
 * -------
 * size_t                         - number of taps required
 *
*/
size_t remez_estimate_num_taps(float_type edge1, float_type ripple1, float_type edge2, float_type ripple2) {  
  size_t num_taps;
  std::vector<float_type> AA = {-4.278e-01,-4.761e-01,0,
                                -5.941e-01,7.114e-02,0,
                                -2.660e-03,5.309e-03,0};

  float_type df = std::abs(edge2 - edge1);
  // convert to log10
  float_type rl = pow(10.0, ripple1/20.0) - 1;
  float_type d1 = log(rl)/log(10.0);
  float_type d2 = -ripple2/20.0;

  std::vector<float_type> AD(3);
  std::vector<float_type> dd = {1, d2, d2*d2};

  size_t off=0;
  for (size_t j=0;j<3;j++) {
    float_type sum=0;
    for (size_t i=0;i<3;i++) {
      sum += AA[i+off]*dd[i];
    }
    off += 3;
    AD[j] = sum;
  }

  float_type D = AD[0] + d1*AD[1] + d1*d1*AD[2];

  float_type fk = 11.01217 + 0.51244*(d1-d2);
  num_taps = (size_t)ceil(D/df - fk*df + 1.0);
  return num_taps;
}

}  // namespace spuce
