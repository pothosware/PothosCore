#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/typedefs.h>
#include <vector>
namespace spuce {
//!  \file
//!  \brief  Class for iir filter design
//!  \author Tony Kirke
//!  \ingroup classes iir

// Only low and high are implemented so far	
enum class filter_type {low,high,bandpass,bandstop};
	
class iir_coeff {
 public:
  //! ord = Filter order
  iir_coeff(long ord = 1, filter_type lp = filter_type::low);
	void resize(long ord);
  //! Destructor
  ~iir_coeff();
  //! Do bilinear transformation
  void bilinear();
  void make_band(float_type center);
  void convert_to_ab();
  void z_root_to_ab(std::vector<std::complex<float_type> >& z);
  void pz_to_ap();
  void ab_to_tf();
  std::vector<float_type> p2_to_poly(const std::vector<std::complex<float_type> >& ab);
  std::vector<float_type> pz_to_poly(const std::vector<std::complex<float_type> >& z);
  void set_zero(const std::complex<float_type>& z, long i) { zeros[i] = z; }
  void set_pole(const std::complex<float_type>& z, long i) { poles[i] = z; }
  std::complex<float_type> get_zero(long i);
  std::complex<float_type> get_pole(long i);
  float_type get_a(long i) const;
  float_type get_b(long i) const;
  float_type get_coeff_a(long i) const;
  float_type get_coeff_b(long i) const;
  float_type max_abs_coeff();
  void apply_gain(float_type g);
  float_type freqz_mag(float_type freq);
  float_type getGain(void) const;
  int getOrder(void) const;
  int getN2(void) const;
  //  int getState(void) const;
  int isOdd(void) const;
  std::vector<float_type> get_a() const;
  std::vector<float_type> get_b() const;
  filter_type get_type(void) const { return lpf; }
  void set_type(filter_type t) { lpf = t; }
  void set_center(float_type c) { center_freq = c; c0 = cos(2*M_PI*c); }
  void set_bandpass_gain();
  float_type get_center() { return center_freq;}
  void print() const;
  void print_pz() const;

 private:
	enum class filter_state {s0,s1,s2,s3,s4};
  std::vector<std::complex<float_type> > poles;
  std::vector<std::complex<float_type> > zeros;
  float_type gain;
  float_type c0;
  float_type center_freq;
  float_type hpf_gain;
  long n2;
  long odd;
  long order;
  filter_state state;
  //long tf_state;
  //long ap_state;
  std::vector<float_type> a_tf;
  std::vector<float_type> b_tf;
  filter_type lpf;
};
// template_instantiations: float_type, std::complex<float_type>
}  // namespace spuce
