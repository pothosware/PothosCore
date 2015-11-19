// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#include <spuce/typedefs.h>
#include <vector>
namespace spuce {

float_type remez_estimate_atten(size_t num_taps, float_type trans_bw, float_type ripple);
float_type remez_estimate_bw(size_t num_taps, float_type ripple, float_type atten);
float_type remez_estimate_weight(float_type pass_ripple_db, float_type stop_atten_db);
/* Estimate the number of taps for a Remez FIR for the input params */
size_t remez_estimate_num_taps(float_type trans_bw, float_type ripple, float_type atten);

}  // namespace spuce
