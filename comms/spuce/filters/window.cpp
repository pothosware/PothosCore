// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
//! \author Tony Kirke
#define _USE_MATH_DEFINES
#include <cmath>
#include <spuce/typedefs.h>
#include <spuce/filters/window.h>
#include <spuce/filters/fir_inv_dft.h>
#include <iostream>
#define TWOPI 2*M_PI
namespace spuce {

inline std::complex<float_type> expj(float_type x) { return (std::complex<float_type>(std::cos(x), std::sin(x))); }
inline float_type coshin(float_type x) { return (log(x + sqrt(x * x - 1.))); }

void real_dft(std::vector<float_type>& y, int n) {
  std::vector<float_type> x(n);

  /*  calculate the w values recursively */
  for (int j = 0; j < n; j++) { x[j] = y[j]; }

  /*  start inverse fft */
  for (int l = 0; l < n; l++) {
    y[l] = 0;
    for (int j = 0; j < n; j++) {
      y[l] += x[j] * cos(TWOPI * l * j / (n));
    }
  }
}

void real_dft(std::vector<std::complex<float_type>>& y, int n) {
  std::vector<std::complex<float_type> > x(n);
	std::complex<float_type> mult;

  /*  calculate the w values recursively */
  for (int j = 0; j < n; j++) {	x[j] = y[j];}

  /*  start inverse fft */
  for (int l = 0; l < n; l++) {
    y[l] = 0;
    for (int j = 0; j < n; j++) {
      mult = x[j] * std::complex<float_type>(cos(TWOPI*l*j/n),sin(TWOPI*l*j/n));
      y[l] += mult;
    }
  }

}
	
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
std::vector<float_type> rectangular(long nf) {
    std::vector<float_type> w(nf);
    for (int i = 0; i < nf; i++) w[i] = 1.0;
    return (w);
}
std::vector<float_type> flattop(long nf)
{
	static const double a0 = 1.0, a1 = 1.93, a2 = 1.29, a3 = 0.388, a4=0.028;
	std::vector<float_type> w(nf);
	for (int i = 0; i < nf; i++) {
		w[i] = a0	-a1*std::cos((2.0*M_PI*i)/(nf-1))
			+a2*std::cos((4.0*M_PI*i)/(nf-1))
			-a3*std::cos((6.0*M_PI*i)/(nf-1))
			+a4*std::cos((8.0*M_PI*i)/(nf-1));
	}
	return w;
}
//!  \ingroup fir
//! \brief hamming window \f$ w(n) = 0.54 - 0.46*cos( 2*\pi*n/(nf-1) )\f$
std::vector<float_type> hamming(long nf) {
    // nf = filter length in samples
    const float_type alpha = 0.54;
    const float_type beta = 0.46;
    std::vector<float_type> w(nf);
    for (int i = 0; i < nf; i++) {
        w[i] = alpha - beta*::cos(TWOPI * i / (nf-1));
    }
    return (w);
}
//
//!  \ingroup fir
//! \brief hann window \f$ w(n) = 0.5( 1 - cos( 2*\pi*n/(nf-1) )\f$
std::vector<float_type> hann(long nf) {
  std::vector<float_type> w(nf);
  // nf = filter length in samples
  for (int i = 0; i < nf; i++) {
      w[i] = 0.5 * (1 - std::cos(TWOPI * i / (nf-1)));
  }
  return (w);
}
//!  \ingroup fir
//! \brief hanning window \f$ w(n) = 0.5( 1 - cos( 2*\pi*(n+1)/(nf+1) )\f$
std::vector<float_type> hanning(long nf) {
  std::vector<float_type> w(nf);
  // nf = filter length in samples
  long even = (nf % 2) == 0;
  float_type xi;
  for (int i = 0; i < nf; i++) {
      xi = i+1;
      w[i] = 0.5 * (1 - std::cos(TWOPI * xi / (nf+1)));
  }
  return (w);
}
//:
//!  \ingroup fir
//! \brief Blackman Window	\f$ w[x] = 0.42 - 0.5*cos(2*\pi*x/nf) + 0.08*cos(2*\pi*x/nf)\f$
std::vector<float_type> blackman(long nf) {
  // nf = filter length in samples
  std::vector<float_type> w(nf);
  for (int i = 0; i < nf; i++) {
    w[i] = 0.42 - 0.5 * std::cos(TWOPI * i /(nf-1)) + 0.08 * std::cos(2 * TWOPI * i / (nf-1));
  }
  return (w);
}
//!  \ingroup fir
//! \brief kaiser window beta calculation for a particular ripple
float_type kaiser_beta(float_type ripple) {
	float_type A = -20.0*log10(ripple);
	float_type beta = (A>50) ? (0.1102*(A-8.7)) : 
		((A<=21) ? 0.0 : (0.5842*pow(A-21.0,0.4) + 0.07886*(A-21.0)));
	return beta;
}
//!  \ingroup fir
//! \brief kaiser window
std::vector<float_type> kaiser(long nf, float_type beta) {
	// nf = filter length in samples
	// beta = parameter of kaiser window
	std::vector<float_type> w(nf);
	float_type bes = 1.0 / io(beta);
	for (int i = 0; i < nf; i++) {
		float_type xi = i - nf/2.0 + 0.5;
		float_type bm = (2.0*xi/(nf - 1));
		float_type val = io(beta * sqrt(1. - bm*bm)) * bes;
		w[i] = val;
	}
	return (w);
}
float_type cheby_poly(int m, float_type a) {
	float_type x;
	int odd = (m%2) ? -1:1;
	if (fabs(a) > 1.0) {
		x = odd*cosh(m * acosh(a));
	} else {
		x = odd*cos(m * acos(a));
	}
	return x;
}
//!  \ingroup fir
//!  \brief dolph chebyshev window design
std::vector<float_type> cheby(int nf, float_type r_db) {
	/*! parameters
		- nf = filter length in samples
		- r_db = Sidelobe level attenuation
	*/
	float_type r = pow(10.0,r_db/20.0);
	float_type x0 = cosh(acosh(r)/(nf-1.0));
	std::vector<float_type> w(nf);

	for (int i = 0; i < nf; i++) {
		float_type a = fabs(x0*cos(i*M_PI/nf));
		w[i] = cheby_poly(nf-1,a);
		if (nf%2 == 0) {
			if (i>nf/2) w[i] *= -1;
		}
	}
	
	if (nf%2 == 0) {
		// For even, need complex input to fft
		std::vector<std::complex<float_type>> cw(nf);
		for (int j = 0; j < nf; j++) cw[j] = w[j] * std::complex<float_type>(cos(M_PI*j/nf),sin(M_PI*j/nf));
		real_dft(cw,nf);
		for (int i = 0; i < nf; i++) w[i] = real(cw[i]);
	} else {
		real_dft(w,nf);
	}
	
	if (nf%2 == 0) {
		for (int i=0;i<nf/2;i++) w[i+nf/2] = w[i];
		for (int i=0;i<nf/2;i++) w[nf/2-1-i] = w[i+nf/2];
	} else {
		for (int i=0;i<nf/2+1;i++) w[nf-1-i] = w[nf/2-i];
		for (int i=0;i<nf/2;i++) w[nf/2-1-i] = w[nf/2+1+i];
	}
	float_type max_v=0;
	// normalize
	for (int i = 0; i < nf; i++) if (fabs(w[i]) > max_v) max_v = fabs(w[i]);
	for (int i = 0; i < nf; i++) w[i] /= max_v;
	return (w);
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
