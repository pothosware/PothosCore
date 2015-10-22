// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
//! \author Tony Kirke
#include <spuce/typedefs.h>
#include <spuce/dsp_functions/fliplr.h>
#include <spuce/dsp_functions/convolve.h>
#include <spuce/dsp_functions/partial_convolve.h>
#include <spuce/filters/iir_coeff.h>
#include <spuce/filters/find_roots.h>
#include <cfloat>
#include <vector>
#include <iostream>
namespace spuce {
std::vector<float_type> iir_coeff::get_a() const {
  std::vector<float_type> p;
  for (int i = 0; i < a_tf.size(); i++) { p.push_back(a_tf[i]); }
  return p;
}
std::vector<float_type> iir_coeff::get_b() const {
  std::vector<float_type> p;
  for (int i = 0; i < b_tf.size(); i++) { p.push_back(b_tf[i]); }
  return p;
}
void iir_coeff::print() const {
  std::cout << "A = {";
  for (int i = 0; i < a_tf.size(); i++) { std::cout << a_tf[i] << " "; }
  std::cout << "}\n";
  std::cout << "B = {";
  for (int i = 0; i < b_tf.size(); i++) { std::cout << b_tf[i] << " "; }
  std::cout << "}\n";
}
void iir_coeff::print_pz() const {
  std::cout << "zeros = {";
  for (int i = 0; i < zeros.size(); i++) { std::cout << zeros[i] << " "; }
  std::cout << "}\n";
  std::cout << "poles = {";
  for (int i = 0; i < poles.size(); i++) { std::cout << poles[i] << " "; }
  std::cout << "}\n";
  std::cout << "gain = " << getGain() << "\n";
}

int iir_coeff::isOdd(void) const { return odd; }
int iir_coeff::getOrder(void) const { return order; }
	//int iir_coeff::getState(void) const { return state; }
int iir_coeff::getN2(void) const { return n2; }
float_type iir_coeff::getGain(void) const { return gain; }
void iir_coeff::apply_gain(float_type g) {
    for (int i=0;i<b_tf.size();i++) {
        b_tf[i] *= g;
    }
}
iir_coeff::iir_coeff(long ord, filter_type lp)
    : poles((ord + 1) / 2), zeros((ord + 1) / 2), a_tf(ord + 1), b_tf(ord + 1), lpf(lp) {
  // amax - attenuation at cutoff
  order = ord;
  n2 = (order + 1) / 2;
  odd = (order % 2);
  for (int j = 0; j < n2; j++) {
    poles[j] = std::complex<float_type>(0.0, 0.0);
    zeros[j] = std::complex<float_type>(0.0, 0.0);
  }
  state = filter_state::s0;  // un-initialized
  //tf_state = 0;
  //ap_state = 0;
}
//! Destructor
iir_coeff::~iir_coeff() {}
// bilinear
void iir_coeff::bilinear() {
  hpf_gain = 1.0;
  if (odd) {
    hpf_gain = 1.0 + real(poles[0]);
    zeros[0] = ((float_type)1.0 - zeros[0]) / ((float_type)1.0 + zeros[0]);
    poles[0] = ((float_type)1.0 - poles[0]) / ((float_type)1.0 + poles[0]);
  }
  for (int j = odd; j < n2; j++) {
    zeros[j] = ((float_type)1.0 - zeros[j]) / ((float_type)1.0 + zeros[j]);
    poles[j] = ((float_type)1.0 - poles[j]) / ((float_type)1.0 + poles[j]);
  }
  state = filter_state::s2;  // in Z-domain now!
}
void iir_coeff::convert_to_ab() {
  float_type hpf_z_gain = 0;
  float_type hpf_p_gain = 0;
  float_type z_gain = 0;
  float_type p_gain = 0;
  gain = (float_type)1.0;

  z_root_to_ab(zeros);
  z_gain = gain;
  hpf_z_gain = hpf_gain;
  gain = 1.0;
  hpf_gain = 1.0;

  z_root_to_ab(poles);
  p_gain = gain;
  hpf_p_gain = hpf_gain;
  gain /= z_gain;
  if (odd) gain *= 0.5 * (1.0 - real(poles[0]));
  hpf_gain = hpf_p_gain / hpf_z_gain;

  if (lpf == filter_type::high) gain = hpf_gain;

  state = filter_state::s3;  // in Z-domain 2nd order A/B coefficients
  a_tf = p2_to_poly(poles);
  b_tf = p2_to_poly(zeros);
  // Apply gain to b coefficents
  apply_gain(gain);
  
}
void iir_coeff::ab_to_tf() {
  a_tf = p2_to_poly(poles);
  b_tf = p2_to_poly(zeros);
  state = filter_state::s3;  // in Z-domain 2nd order A/B coefficients
}
void iir_coeff::z_root_to_ab(std::vector<std::complex<float_type> >& z) {
  for (int j = odd; j < n2; j++) {
    gain *= (std::norm(z[j]) - 2 * real(z[j]) + 1.0);
    hpf_gain *= (std::norm(z[j]) + 2 * real(z[j]) + 1.0);
    z[j] = std::complex<float_type>(-2 * real(z[j]), std::norm(z[j]));
  }
  if (gain == 0.0) gain = 1.0;
  state = filter_state::s3;  // in Z-domain 2nd order A/B coefficients
}
// Takes poles or zeros and creates a polynomial transfer function
std::vector<float_type> iir_coeff::pz_to_poly(const std::vector<std::complex<float_type> >& z) {
  std::vector<float_type> p2(3);
  std::vector<float_type> p(order + 1);
  std::vector<float_type> tf(order + 1);
  float_type td;

  p[0] = 1;
  p2[0] = 1;
  long m = 1;

  for (int j = 0; j < n2; j++) {
    td = 1 - 2 * real(z[j]) + std::norm(z[j]);
    p2[1] = (1 - std::norm(z[j])) / td;
    p2[2] = 2.0 * imag(z[j]) / td;
    tf = partial_convolve(p, p2, m, 3);
    m += 2;
    for (int i = 0; i < m; i++) p[i] = tf[i];
  }
  //tf_state = 1;
  return (tf);
}
// Takes 'n' 2nd order polynomials of the form 1+a*z + b*z^2
// where a and b are packed into a std::complex float_type as std::complex<float_type>(a,b)
// and convolves them all together as 1 polynomial
std::vector<float_type> iir_coeff::p2_to_poly(const std::vector<std::complex<float_type> >& ab) {
  std::vector<float_type> tf;
  std::vector<float_type> p2(3);
  std::vector<float_type> p(order + 1);

  p[0] = 1;
  p2[0] = 1;
  long m = 1;
  if (odd) {
    p2[1] = -real(ab[0]);
    p2[2] = 0;
    tf = partial_convolve(p, p2, m, 2);
    m += 1;
    for (int i = 0; i < m; i++) p[i] = tf[i];
  }
  for (int j = odd; j < n2; j++) {
    p2[1] = real(ab[j]);
    p2[2] = imag(ab[j]);
    tf = partial_convolve(p, p2, m, 3);
    m += 2;
    for (int i = 0; i < m; i++) { p[i] = tf[i]; }
  }
  return tf;
}
float_type iir_coeff::get_a(long i) const {
  if (i < order + 1) {
    return (a_tf[i]);
  } else {
    return (0);
  }
}
float_type iir_coeff::get_coeff_a(long i) const {
  if (i < order) {
    if (i % 2 == 0)
      return (real(poles[i / 2]));
    else
      return (imag(poles[i / 2]));
  } else {
    return (0);
  }
}
float_type iir_coeff::get_coeff_b(long i) const {
  if (i < order) {
    if (i % 2 == 0)
      return (real(zeros[i / 2]));
    else
      return (imag(zeros[i / 2]));
  } else {
    return (0);
  }
}
float_type iir_coeff::get_b(long i) const {
  if (i < order + 1) {
    return (b_tf[i]);
  } else {
    return (0);
  }
}
std::complex<float_type> iir_coeff::get_pole(long i) {
  if (i <= n2)
    return (poles[i]);
  else
    return (std::complex<float_type>(0.0, 0.0));
}
std::complex<float_type> iir_coeff::get_zero(long i) {
  if (i <= n2)
    return (zeros[i]);
  else
    return (std::complex<float_type>(0.0, 0.0));
}
void iir_coeff::pz_to_ap() {
  int m = 2 * order - 1;
  typedef std::vector<float_type> Array;
  typedef std::vector<std::complex<float_type> > CArray;
  Array fa;
  Array d2(m);
  Array p2(m);
  Array r(m);
  Array q(m);
  CArray rq;
  CArray h1(m);
  CArray h2(m);
  float_type divtmp;
  float_type tmp;
  int np, nq;
  int i, j;

  // Convert from poles and zeros to 2nd order section coefficients
  //  root_to_ab(zeros);
  //  root_to_ab(poles);

  // Get overall A and B transfer functions
  //  p2_to_poly(zeros, b_tf, n2);
  //  p2_to_poly(poles, a_tf, n2);

  // Convert from poles and zeros to polynomial transfer functions
  b_tf = pz_to_poly(zeros);
  a_tf = pz_to_poly(poles);

  // Now start real work
  p2 = convolve(b_tf, b_tf);
  fa = fliplr(a_tf);
  d2 = convolve(fa, a_tf);

  // B*B - A*fliplr(A)
  for (j = 0; j < m; j++) { r[j] = p2[j] - d2[j]; }

  // Appendix IEEE assp-34, no 2, april 1986, page 360
  q[0] = std::sqrt(r[0]);
  q[1] = r[1] / (2 * q[0]);

  divtmp = 0.5 / q[0];
  for (j = 2; j < m; j++) {
    for (tmp = 0, i = 2; i < j; i++) tmp += q[i] * q[j - i];
    q[j] = (r[j] - tmp) * divtmp;
  }

  for (j = 0; j < m; j++) q[j] += b_tf[j];

  rq = find_roots(q, m);

  np = nq = 0;
  for (j = 0; j < m; j++) {
    if (std::norm(rq[j]) >= (float_type)1.0) {
      h1[nq++] = rq[j];
    } else {
      h2[np++] = rq[j];
    }
  }
  // We now roots for H1 sections in h1 and
  //              for H2 sections in h2 and

  // We convert from each root pair to an allpass section
  // with coefficients

  // Save these coefficients for transfer to IIR implemented as
  // allpass sections

  //ap_state = 1;
  state = filter_state::s4;
}
float_type iir_coeff::max_abs_coeff() {
  float_type maxv = 0;
  for (int j = 0; j < n2; j++) {
    if (std::fabs(real(poles[j])) > maxv) maxv = std::fabs(real(poles[j]));
    if (std::fabs(imag(poles[j])) > maxv) maxv = std::fabs(imag(poles[j]));
    if (std::fabs(real(zeros[j])) > maxv) maxv = std::fabs(real(zeros[j]));
    if (std::fabs(imag(zeros[j])) > maxv) maxv = std::fabs(imag(zeros[j]));
  }
  return (maxv);
}

// Get frequency response at freq
float_type iir_coeff::freqz_mag(float_type freq) {
  int i;
  std::complex<float_type> z(1, 0);
  std::complex<float_type> z_inc = std::complex<float_type>(std::cos(freq), std::sin(freq));
  std::complex<float_type> nom(0);
  std::complex<float_type> den(0);
  for (i = 0; i < order + 1; i++) {
    nom += z * b_tf[i];
    den += z * a_tf[i];
    z *= z_inc;
  }
  return std::sqrt(std::norm(nom / den));
}
}  // namespace spuce
