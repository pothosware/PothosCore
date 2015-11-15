// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <cmath>
#include <spuce/typedefs.h>
#include <spuce/filters/raised_cosine_imp.h>
namespace spuce {
float_type raised_cosine_imp(float_type alpha, float_type xin, float_type r, long num_taps)
//------------------------------------------------------------------------------------------------
// Calculates the raised cosine pulse shape given the excess bandwidth value alpha and the index.
//------------------------------------------------------------------------------------------------
{
  float_type x1, x2, rc1;
  float_type rate = 0.5*r;
  float_type xindx = xin - num_taps / 2;
  x1 = M_PI * xindx / rate;
  x2 = 1 - (4 * alpha * alpha * (xindx / rate) * (xindx / rate));
  if (x1 == 0) return (1);
  if (x2 == 0) {
    x2 = 8 * alpha * (xindx / rate) * (xindx / rate);
    rc1 = sin(x1) * sin(alpha * x1) / x2;
  } else {
    rc1 = (sin(x1) * cos(alpha * x1)) / (x1 * x2);
  }
  return (rc1);
}
}  // namespace spuce
