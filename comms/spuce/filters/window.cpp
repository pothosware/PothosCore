// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <cmath>
#include <spuce/typedefs.h>
#include <spuce/filters/window.h>
#include <iostream>
#define TWOPI 2*M_PI
namespace spuce {

inline std::complex<float_type> expj(float_type x) { return (std::complex<float_type>(std::cos(x), std::sin(x))); }
inline float_type coshin(float_type x) { return (log(x + sqrt(x * x - 1.))); }

//!  \author Tony Kirke,  Copyright(c) 2001
//!
//! \ingroup fir
//! function:  io
//! \brief bessel function for kaiser window
float_type io(float_type x) {
  const float_type t = 1.e-08;
  float_type y = 0.5 * x;
  float_type e = 1.0;
  float_type de = 1.0;
  int i;
  float_type xi;
  float_type sde;
  for (i = 1; i < 26; i++) {
    xi = i;
    de *= y / xi;
    sde = de * de;
    e += sde;
    if ((e * t - sde) > 0) break;
  }
  return (e);
}
//!  \ingroup fir
//! \brief hamming window \f$ w(n) = alpha + beta*cos( 2*\pi*(n-1)/(nf-1) )\f$
std::vector<float_type> hamming(long nf, float_type alpha, float_type beta) {
  // nf = filter length in samples
  // alpha = constant of window
  // beta = constant of window--generally beta=1-alpha
  std::vector<float_type> w(nf);
  long odd = nf % 2;
  float_type xi;
  for (int i = 0; i < nf; i++) {
    xi = i;
    if (odd) xi += 0.5;
    w[i] = alpha - beta*::cos(TWOPI * xi / nf);
  }
  return (w);
}
//
//!  \ingroup fir
//! \brief hanning window \f$ w(n) = 0.5( 1 - cos( 2*\pi*n/(nf-1) )\f$
std::vector<float_type> hanning(long nf) {
  std::vector<float_type> w(nf);
  // nf = filter length in samples
  // alpha = constant of window
  // beta = constant of window--generally beta=1-alpha
  long odd = nf % 2;
  float_type xi;
  for (int i = 0; i < nf; i++) {
    xi = i;
    if (odd) xi += 0.5;
    w[i] = 0.5 * (1 - ::cos(TWOPI * xi / nf));
  }
  return (w);
}
//:
//!  \ingroup fir
//! \brief Blackman Window	\f$ w[x] = 0.42 - 0.5*cos(2*\pi*x/nf) + 0.08*cos(2*\pi*x/nf)\f$
std::vector<float_type> blackman(long nf) {
  // nf = filter length in samples
  // alpha = constant of window
  // beta = constant of window--generally beta=1-alpha
  std::vector<float_type> w(nf);
  long odd = nf % 2;
  float_type xi;
  for (int i = 0; i < nf; i++) {
    xi = i;
    if (odd) xi += 0.5;
    w[i] = 0.42 - 0.5 * ::cos(TWOPI * xi / nf) + 0.08 * ::cos(2 * TWOPI * xi / nf);
  }
  return (w);
}
//!  \ingroup fir
//! \brief kaiser window
std::vector<float_type> kaiser(long nf, float_type tw, float_type ripple) {
  // nf = filter length in samples
	float_type tww = tw*M_PI;
	float_type A = -20.0*log10(ripple);
	int M = (A>21.0) ? (int)ceil((A-7.95)/(2.285*tww)) : (int)ceil(5.79/tww);
	float_type beta = (A>50) ? (0.1102*(A-8.7)) : 
		((A<=21) ? 0.0 : (0.5842*pow(A-21.0,0.4) + 0.07886*(A-21.0)));
  // beta = parameter of kaiser window
  std::vector<float_type> w(nf);
  float_type bes = 1.0 / io(beta);
  long odd = nf % 2;
  for (int i = 0; i < nf/2; i++) {
		float_type xi = i;
    if (odd)   xi = i + 0.5;
		float_type bm = (2*xi/M - 1);
    w[i] = io(beta * sqrt(1. - bm*bm)) * bes;
    w[nf - 2 - i] = w[i];
  }
  //for (int i = 0; i < nf; i++) std::cout << "w[" << i << "] = " << w[i] << "\n";
  return (w);
}

//!  \ingroup fir
//!  \brief dolph chebyshev window design
std::vector<float_type> cheby(long nf, long n, long ieo, float_type dp, float_type df, float_type x0) {
  /*! parameters
        - nf = filter length in samples
        - w = window array of size n
        - n = half length of filter = (nf+1)/2
        - ieo = even-odd indicator--ieo=0 for nf even
        - dp = window ripple on an absolute scale
        - df = std::normalized transition width of window
        - x0 = window parameter related to transition width
        - xn = nf-1
  */
  std::vector<float_type> w(nf);
  float_type xn = nf - 1;
  float_type fnf = nf;
  float_type alpha = (x0 + 1.) / 2.;
  float_type beta = (x0 - 1.) / 2.;
  float_type c2 = xn / 2.;
  float_type p;
  std::complex<float_type> pie;
  float_type twn;
  float_type xi;
  float_type sum;
  float_type xj;
  float_type f;
  float_type x;
  float_type c1;
  int j;

  for (int i = 1; i <= nf; i++) {
    xi = i - 1;
    f = xi / fnf;
    x = alpha * ::cos(TWOPI * f) + beta;
    if ((fabs(x) - 1.) > 0) {
      p = dp * ::cos(c2 * acos(x));
    } else {
      p = dp * cosh(c2 * coshin(x));
    }
    pie = std::complex<float_type>(0.0, p);
    //
    // for even length filters use a one-half sample delay
    // also the frequency response is antisymmetric in frequency
    //
    if (ieo != 1) {
      pie = std::complex<float_type>(p * expj(-f * M_PI));
      if (i > (nf / 2 + 1)) pie *= -1;
    }
    // use dft to give window
    twn = TWOPI / fnf;
    for (i = 1; i < n + 1; i++) {
      xi = i - 1;
      sum = 0.;
      for (j = 1; j < nf + 1; j++) {
        xj = j - 1;
        //	  sum += real(pie[j])*cos(twn*xj*xi) + imag(pie[j])*sin(twn*xj*xi);
        sum += real(pie) * ::cos(twn * xj * xi) + imag(pie) * sin(twn * xj * xi);
      }
      w[i] = sum;
    }
    c1 = w[1];
    for (i = 1; i < n + 1; i++) w[i] /= c1;
  }
  return (w);
}
//!  \ingroup fir
//! \brief chebyshev window
//! subroutine to generate chebyshev window parameters when
//! one of the three parameters nf,dp and df is unspecified???
void chebc(float_type nf, float_type dp, float_type df, float_type n, float_type x0) {
  //
  // nf = filter length (in samples)
  // dp = filter ripple (absolute scale)
  // df = std::normalized transition width of filter
  // n = (nf+1)/2 = filter half length
  // x0 = (3-c0)/(1+c0) with c0=cos(pi*df) = chebyshev window constant
  // xn = nf-1
  //
  float_type xn;
  float_type c1, c0, c2, x;
  if (nf == 0) {
    // dp,df specified, determine nf
    c1 = coshin((1. + dp) / dp);
    c0 = ::cos(M_PI * df);
    x = 1. + c1 / coshin(1. / c0);
    // increment by 1 to give nf which meets or exceeds specs on dp and df
    nf = x + 1.0;
    n = (nf + 1) / 2;
    xn = nf - 1;
    x0 = (3. - ::cos(TWOPI * df)) / (1. + ::cos(TWOPI * df));
    return;
  }
  if (df == 0) {
    // nf,dp specified, determine df
    xn = nf - 1;
    c1 = coshin((1. + dp) / dp);
    c2 = cosh(c1 / xn);
    df = acos(1. / c2) / M_PI;
    x0 = (3. - ::cos(TWOPI * df)) / (1. + ::cos(TWOPI * df));
  }
  // nf,df specified, determine dp
  xn = nf - 1;
  c0 = ::cos(M_PI * df);
  c1 = xn * coshin(1. / c0);
  dp = 1. / (cosh(c1) - 1.);
  x0 = (3. - ::cos(2. * M_PI * df)) / (1. + ::cos(2. * M_PI * df));
}
std::vector<float_type> bartlett(long nf) {
  std::vector<float_type> w(nf);
  for (int i = 0; i < nf / 2; i++) {
    float_type win = 2.0 * i / (nf - 1);
    w[i] = win;
    w[nf - 1 - i] = win;
  }
  if (nf % 2 == 1) {
		w[nf / 2] = 1.0;
	}
  return (w);
}
}  // namespace spuce
