// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#include <spuce/filters/farrow_upsampler.h>
namespace spuce {
farrow_upsampler::farrow_upsampler(int order) : INTERP(order) { time_offset = 0; }
int farrow_upsampler::need_sample(float_type t_inc, float_type x) {
  if (time_offset + t_inc >= 1.0) {
    INTERP.input(x);
    return (1);
  } else {
    return (0);
  }
}
float_type farrow_upsampler::output(float_type t_inc) {
  time_offset += t_inc;
  if (time_offset >= 1.0) { time_offset -= 1.0; }
  return (INTERP.rephase(time_offset));
}
}  // namespace spuce
