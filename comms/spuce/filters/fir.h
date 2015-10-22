#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/fir_coeff.h>
#include <cassert>
#include <vector>
#include <spuce/base_type.h>
#include <spuce/mixed_type.h>
namespace spuce {
//! \file
//! \brief  Template Class for Modeling a Finite Impulse Response filter.
//
//!  Template works for float_type, long, std::complex, etc
//!  Taps initialized to zeros.
//! \author Tony Kirke
//! \ingroup double_templates fir
//! \image html fir.gif
//! \image latex fir.eps
template <class Numeric, class Coeff = float_type> class fir {
 public:
  //! Set tap weights
  void settap(long i, Coeff tap) { coeff[i] = tap; }
  void settaps(fir_coeff<Coeff>& c) {
    for (int i = 0; i < num_taps; i++) coeff[i] = c.coeff[i];
  }
  long number_of_taps() const { return (num_taps); }
  Coeff gettap(long i) { return (coeff[i]); }
  //! Reset
  void reset() {
    for (int i = 0; i < num_taps; i++) z[i] = (Numeric)0;
    output = (Numeric)0;
  }
  //! Get sum of coefficients
  Coeff coeff_sum() {
    int i;
    Coeff s;
    for (s = 0, i = 0; i < num_taps; i++) s += coeff[i];
    return (s);
  }
  void normalize_gain() {
    float_type c = 1.0 / coeff_sum();
    for (int i = 0; i < num_taps; i++) coeff[i] *= c;
  }
  void make_hpf() {
    bool inv = true;
    for (int i = 0; i < num_taps; i++) {
      if (inv) coeff[i] *= -1;
      inv = !inv;
    }
  }

  //! Get current output
  Numeric out() { return (output); }
  //! Clock in new sample & compute current output
  Numeric check(long i) { return (z[i]); }
  ~fir(void) {}
  //! Constructor
  fir(void) { ; }
  //! Constructor
  fir(long n) : coeff(n), z(n), num_taps(n) {
		assert(n > 0);
    int i;
    if (n > 0) {
      for (i = 0; i < n; i++) z[i] = (Numeric)0;
      for (i = 0; i < n; i++) coeff[i] = (Coeff)0;
    }
  }
  //! Constructor
  fir(fir_coeff<Coeff> C) : coeff(C.num_taps), z(C.num_taps) {
    int i;
    int n = num_taps = C.num_taps;
    if (n > 0) {
      coeff.resize(n);
      z.resize(n);
      for (i = 0; i < n; i++) z[i] = (Numeric)0;
      for (i = 0; i < n; i++) coeff[i] = C.coeff[i];
    }
  }
  void set_coeffs(fir_coeff<Coeff> C) {
    int i;
    int n = num_taps = C.num_taps;
    if (n > 0) {
      coeff.resize(n);
      z.resize(n);
      for (i = 0; i < n; i++) z[i] = (Numeric)0;
      for (i = 0; i < n; i++) coeff[i] = C.coeff[i];
    }
  }
  //! Set size of Filter
  void set_size(long n) {
		if (get_size() == n) {
			// size already is correct
		} else {
			num_taps = n;
			if (n > 0) {
				coeff.resize(n);
				z.resize(n);
				for (int i = 0; i < n; i++) z[i] = (Numeric)0;
				for (int i = 0; i < n; i++) coeff[i] = (Coeff)0;
			} else {
				coeff.resize(0);
				z.resize(0);
			}
		}
  }
  long get_size(void) { return (num_taps); }
  //!  Constructor that gets coefficients from file (requires fir.cpp)
  fir(const char* file) { read_taps(file); }
  // Assign fir of one type to another
  //  template <class T1> fir<T> operator= (fir<T1> const&);
  //! Update filter by inputting 1 sample and returning convolved output sample.
  Numeric clock(Numeric in) { return (update(in)); }
  Numeric update(Numeric in) {
    int i;
    sum_type sum(0);
    // Update history of inputs
    for (i = num_taps - 1; i > 0; i--) z[i] = z[i - 1];
    // Add new input
    z[0] = in;
    // Perform FIR
    for (i = 0; i < num_taps; i++) sum = sum + coeff[i] * z[i];

    output = (sum);
    return (output);
  }
  void process(const std::vector<Numeric>& in, std::vector<Numeric>& out) {
    // Update history of inputs
    for (int j = 0; j < in.size(); j++) {
      for (int i = num_taps - 1; i > 0; i--) z[i] = z[i - 1];
      // Add new input
      z[0] = in[j];
      // Perform FIR
      sum_type sum(0);
      for (int i = 0; i < num_taps; i++) sum = sum + coeff[i] * z[i];
      out[j] = (sum);
    }
  }
  // Tapped delay line uses previous outputs (to behave like an IIR)
  Numeric iir(Numeric in) {
    typename mixed_type<Numeric, Coeff>::dtype sum;
    int i;
    for (sum = 0, i = 0; i < num_taps; i++) sum = sum + coeff[i] * z[i];
    // Update history of outputs
    for (i = num_taps - 1; i > 0; i--) z[i] = z[i - 1];

    output = (sum + in);
    // Add new output to delay line
    z[0] = output;
    return (output);
  }
  int read_taps(const char* file) {
    fir_coeff<Coeff> c(num_taps);
    int r = c.read_taps(file);
    for (int i = 0; i < num_taps; i++) coeff[i] = c.coeff[i];
    return (r);
  }
  void print(void) {
    fir_coeff<Coeff> c(num_taps);
    for (int i = 0; i < num_taps; i++) c.settap(i, coeff[i]);
    c.print();
  }

  template <class N, class C> friend std::vector<C> get_taps(const fir<N, C>& x);
  template <class N, class C> friend std::vector<N> get_input(const fir<N, C>& y);
  void settap(std::vector<Coeff> z) {
    for (int i = 0; i < num_taps; i++) coeff[i] = z[i];
  }
	Coeff get_coeff(int i) const { return coeff[i];}
	
 private:
  std::vector<Coeff> coeff;
  std::vector<Numeric> z;
  long num_taps;
  Numeric output;
  typedef typename base_type<Numeric>::btype Numeric_base;
  typedef typename mixed_type<Numeric, Coeff>::dtype sum_type;
};

template <class Numeric, class Coeff> std::vector<Coeff> get_taps(const fir<Numeric, Coeff>& f) {
  long N = f.num_taps;
  std::vector<Coeff> V(N);
  for (int i = 0; i < N; i++) V[i] = f.coeff[i];
  return (V);
}
template <class Numeric, class Coeff> std::vector<Numeric> get_input(const fir<Numeric, Coeff>& f) {
  long N = f.num_taps;
  std::vector<Numeric> V(N);
  for (int i = 0; i < N; i++) V[i] = f.z[i];
  return (V);
}
}  // namespace spuce
