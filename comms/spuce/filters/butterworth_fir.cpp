// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <cmath>
#include <spuce/filters/fir_coeff.h>
#include <spuce/typedefs.h>
#include <spuce/filters/butterworth_fir.h>
namespace spuce {
//! \file
//! \brief calculates the sampled butterworth (max flat) filter impulse response
void butterworth_fir(fir_coeff<float_type>& butfir, float_type spb) {
  /*!

    With B(p) the butterworth response we have:

  \f$ B(p)  = \displaystyle\sum_{k=1}^{N/2} \frac{\lambda(k)*(1+\alpha(k)*p/wc)}{1+2*cos(\beta(k))*p/wc + (p/wc)^2)}.\f$


             with<p>
             N = Order of Butterworth filter (always even)<p>
  \f$	   \beta(k) = (2 * k - 1) / (2 * N)         k = 1, 2, ..  N/2 \f$<p>
  \f$	   \alpha(k) = sin((N/2 - 1) * \beta(k)) / sin(N * \beta(k) / 2) \f$<p>

  \f$ \lambda(p)  = \frac{\sin(N*\beta(k)/2)}{sin(\beta(k))}. \displaystyle\prod_{m=1,m!=k}^{N/2} 2*cos(\beta(k) -
  cos(\beta(m))) \f$


            The impulse response of B(p) can be found by realizing that:

  \f$ \frac{p+a}{(p+a)^2 + w^2} <-> e^{-at} * cos(w*t) \f$

  \f$ \frac{w}{(p+a)^2 + w^2} <-> e^{-at} * sin(w*t) \f$

  and that B(p) can be written as a linear combination of the these two
  expressions:

  \f$ \frac{x*(p+a)+y*w}{(p+a)^2 + w^2} <-> e^{-at}*x*cos(w*t) + y* sin(w*t) \f$

  We find after some algebra:<p>
  \f$ x(k) = \alpha(k) \f$<p>
  \f$ w(k) = \sqrt{1 - cos(\beta(k)) * cos((\beta(k)))} \f$<p>
  \f$ a(k) = cos(\beta(k)) \f$<p>
  \f$ y(k) = (1 - \alpha(k) * cos(\beta(k)) / w(k) \f$<p>
  Also used is the time scaling rule for Fourier transforms:
  |a| * y(at) <--> Y(f/a)

  */
  int end, i, i2, j;
  float_type x, xend, t;
  int taps = butfir.number_of_taps();
  int spbi = (int)floor(1.0 / spb + 0.5);
  int ord = (int)floor(taps / spbi + 0.5);
  if (ord % 2) ord += 1;  // make even
  end = ord / 2;
  xend = (float_type)end;
  std::vector<float_type> beta(end);
  std::vector<float_type> alpha(end);
  std::vector<float_type> ak(end);
  std::vector<float_type> xk(end);
  std::vector<float_type> yk(end);
  std::vector<float_type> wk(end);
  std::vector<float_type> lamda(end);

  for (i = 0; i < end; i++) {
    x = (float_type)i + 1.0;
    beta[i] = M_PI * (2.0 * x - 1.0) / (2.0 * ord);
    lamda[i] = sin(xend * beta[i]) / sin(beta[i]);
    alpha[i] = sin((xend - 1.0) * beta[i]) / sin(xend * beta[i]);
    ak[i] = cos(beta[i]);
    wk[i] = sqrt(1 - ak[i] * ak[i]);
    xk[i] = alpha[i];
    yk[i] = (1.0 - alpha[i] * ak[i]) / wk[i];
  }

  for (i = 0; i < end; i++)
    for (i2 = 0; i2 < end; i2++)
      if (i2 != i) lamda[i] /= (2.0 * (ak[i] - cos(beta[i2])));

  float_type ht;
	float_type sum=0;
	float_type coeff;
  for (j = 0; j < taps; j++) {
    ht = 0.0;
    t = 2*M_PI * j / (float_type)spbi;
    for (i = 0; i < end; i++) ht += lamda[i] * ::exp(-t * ak[i]) * (xk[i] * cos(t * wk[i]) + yk[i] * sin(t * wk[i]));
		coeff = 2.0*M_PI*ht;
		sum += coeff;
    butfir.settap(j,coeff);
  }
	// Normalize
  for (j = 0; j < taps; j++) {
		butfir.settap(j,butfir.gettap(j)/sum);
	}
}
}  // namespace spuce
