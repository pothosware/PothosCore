// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <cmath>
#include <spuce/typedefs.h>
#include <spuce/filters/root_raised_cosine_imp.h>
namespace spuce {
float_type root_raised_cosine_imp(float_type alpha, float_type xin, float_type r, long num_taps)
//----------------------------------------------------------------------------------------------------
// Calculates the square root raised cosine pulse shape given the bandwidth value alpha and the index.
//----------------------------------------------------------------------------------------------------
{
  float_type x1, x2, x3;
  float_type nom, denom;
  float_type rate = 0.5*r;
  float_type xindx = xin - num_taps / 2;
  x1 = M_PI * xindx / rate;
  x2 = 4 * alpha * xindx / rate;
  x3 = x2 * x2 - 1;
  if (x3 != 0) {
    if (x1 != 0) {
      nom = cos((1 + alpha) * x1);
      nom += sin((1 - alpha) * x1) / (4 * alpha * xindx / rate);
    } else {
      nom = cos((1 + alpha) * x1);
      nom += (1 - alpha) * M_PI / (4 * alpha);
    }
    denom = x3 * M_PI;
  } else {
    if (alpha == 1) return (-1);
    x3 = (1 - alpha) * x1;
    x2 = (1 + alpha) * x1;
    nom = sin(x2) * (1 + alpha) * M_PI - cos(x3) * ((1 - alpha) * M_PI * rate) / (4 * alpha * xindx) +
          sin(x3) * rate * rate / (4 * alpha * xindx * xindx);
    denom = -32 * M_PI * alpha * alpha * xindx / rate;
  }
  return (4 * alpha * nom / denom);
}

}  // namespace spuce
