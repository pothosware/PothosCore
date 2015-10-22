// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#include <spuce/filters/elliptic_iir.h>
#include <cmath>
#include <cfloat>
#define MACHEP 2.0 * FLT_MIN
using namespace std;
namespace spuce {
void elliptic_iir(iir_coeff& filt, float_type fcd, float_type ripple, float_type stopattn, float_type bw) {
  float_type m1, m2;
  float_type a, m, Kk1, Kpk1, k, wr, fstp, Kk, u;
  const float_type ten = 10.0;
  auto order = filt.getOrder();
  float_type epi = pow(ten, (ripple / ten));
  epi = sqrt(epi - 1.0);
	float_type fstop = fcd + bw;
  //! wca - pre-warped angular frequency
  float_type wca  = tan(M_PI * fcd);
  float_type wc  =  (filt.get_type()==filter_type::low) ? tan(M_PI * fcd) : tan(M_PI*(0.5-fcd));
  //! if stopattn < 1 dB assume it is stopband edge instead
  if (stopattn > 1.0) {
    a = pow(ten, (stopattn / ten));
    m1 = epi * epi / (a - 1.0);
    m2 = 1 - m1;
    Kk1 = ellpk(m1);
    Kpk1 = ellpk(m2);
    u = Kpk1 / (order * Kk1);
    k = msqrt(u);
    wr = 1.0 / k;
    fstp = atan(wca * wr) / M_PI;
  } else {
    fstp = stopattn;
  }

  wr = tan(fstp * M_PI) / wca;
  if (wr < 0.0) wr = -wr;
	// Selectivity factor
  k = 1.0 / wr;
  m = k * k;
  Kk = ellpk(m);
  u = lamda_plane(k, m, order, epi);
  s_plane(filt, order, u, m, k, Kk, wc);

  filt.bilinear();
  filt.convert_to_ab();
  // Must scale even order filter by this factor
  float_type gain = pow(ten, -(ripple / 20.0));
  if (!filt.isOdd()) filt.apply_gain(gain);
}
//! get roots in Lamda plane
float_type lamda_plane(float_type k, float_type m, int n, float_type eps) {
  float_type m1;
  float_type Kk;
  float_type Kpk;
  float_type Kk1;
  float_type u;
  float_type phi;

  Kk = ellpk(m);
  Kpk = ellpk(1 - m);
  u = (n * Kpk / Kk);
  m1 = msqrt(u);
  m1 *= m1;
  Kk1 = ellpk(m1);
  phi = atan(1.0 / eps);
  u = Kk * ellik(phi, 1.0 - m1) / (n * Kk1);
  return u;
}
//! calculate s plane poles and zeros
void s_plane(iir_coeff& filt, int n, float_type u, float_type m, float_type k, float_type Kk, float_type wc) {
  float_type b;
  float_type sn1, cn1, dn1;
  float_type sn, cn, dn;
  float_type r;
  int i, j;
  float_type kon = Kk / (float_type)n;
  ellpj(u, 1.0 - m, sn1, cn1, dn1);
  for (i = 0; i < (n + 1) / 2; i++) {
    b = (n - 1 - 2 * i) * kon;
    j = (n - 1) / 2 - i;
    ellpj(b, m, sn, cn, dn);
    r = k * sn * sn1;
    r = 1.0 / (cn1 * cn1 + r * r);
    if (filt.get_type()==filter_type::low) {
      if (sn != 0) {
        filt.set_zero(-std::complex<float_type>(0.0, wc / (k * sn)), j);
      } else {
        filt.set_zero(FLT_MAX, j);
      }
      filt.set_pole(-std::complex<float_type>(-wc * cn * dn * sn1 * cn1 * r, wc * sn * dn1 * r), j);
    } else {
      if (sn != 0) {
        filt.set_zero(-std::complex<float_type>(0.0, -1.0 / (wc / (k * sn))), j);
      } else {
        filt.set_zero(0, j);
      }
      filt.set_pole(-1.0 / std::complex<float_type>(-wc * cn * dn * sn1 * cn1 * r, wc * sn * dn1 * r), j);
    }
  }
}
int ellpj(float_type u, float_type m, float_type& sn, float_type& cn, float_type& dn) {
  float_type ai, b, phi, t, twon;
  float_type a[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  float_type c[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int i;

  a[0] = 1.0;
  b = sqrt(1.0 - m);
  c[0] = sqrt(m);
  twon = 1.0;
  i = 0;

  while ((std::fabs(c[i] / a[i]) > MACHEP) & (i < 8)) {
    ai = a[i];
    ++i;
    c[i] = (ai - b) / 2.0;
    t = sqrt(ai * b);
    a[i] = (ai + b) / 2.0;
    b = t;
    twon *= 2.0;
  }

  phi = twon * a[i] * u;
  do {
    t = c[i] * sin(phi) / a[i];
    b = phi;
    phi = (asin(t) + phi) / 2.0;
  } while (--i);

  sn = sin(phi);
  t = cos(phi);
  cn = t;
  dn = t / cos(phi - b);
  return (0);
}
//  int ellik(float_type phi,float_type k,float_type *fe,float_type *ee);
//          phi -- argument in radians (pi/2 for complete elliptic integral)
//          k  -- (k) modulus, (0.0 - 1.0)
float_type ellik(float_type phi, float_type k) {
  float_type g, fac, a, b, c, d, a0, b0, d0, r;
  float_type fe;

  g = 0.0;
  a0 = 1.0;
  r = k * k;
  b0 = sqrt(1.0 - r);
  d0 = phi;
  if (k == 1.0) {
    return (log((1.0 + sin(d0)) / cos(d0)));
  } else {
    fac = 1.0;
    for (int n = 1; n < 41; n++) {
      a = 0.5 * (a0 + b0);
      b = sqrt(a0 * b0);
      c = 0.5 * (a0 - b0);
      fac *= 2.0;
      r += (fac * c * c);
      d = d0 + atan((b0 / a0) * tan(d0));
      g += (c * sin(d));
      d0 = d + M_PI * int(d / M_PI + 0.5);
      a0 = a;
      b0 = b;
      if (c < 1.0e-7) break;
    }
    fe = d / (fac * a);
  }
  return (fe);
}
float_type ellpk(float_type k) {
  float_type fac, a, b, c, a0, b0, r;
  float_type fe;
  a0 = 1.0;
  b0 = sqrt(1.0 - k);
  c = sqrt(k);
  if (k == 1.0)
    fe = FLT_MAX;
  else {
    fac = 0.5;
    r = fac * c * c;
    for (int n = 1; n < 41; n++) {
      a = 0.5 * (a0 + b0);
      c = 0.5 * (a0 - b0);
      b = sqrt(a0 * b0);
      fac *= 2.0;
      r += (fac * c * c);
      a0 = a;
      b0 = b;
      if (c < 1.0e-7) break;
    }
    fe = M_PI / (2.0 * a);
  }
  return (fe);
}
//! modulus from ratio of K/K'
float_type msqrt(float_type u) {
  float_type dt1;
  float_type a = 1.0;
  float_type b = 1.0;
  float_type r = 1.0;
  //! see http://www.physik-astro.uni-bonn.de/~dieckman/InfProd/InfProd.html
  //! calculate  theta2 / theta3
  //! m = (theta2/theta3) ^ 4
  float_type q = ::exp(-M_PI * u);
  float_type p = q;

  do {
    r *= p;
    a += 2.0 * r;
    dt1 = r / a;
    r *= p;
    b += r;
    p *= q;
  } while (dt1 > 1.0e-7);

  a = b / a;
  a = 4.0 * sqrt(q) * a * a;
  return (a);
}
}  // namespace spuce
