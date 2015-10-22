#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/typedefs.h>
#include <iostream>
#include <vector>
namespace spuce {
//! \file
//! \brief  Template Class for Modeling a Finite Impulse Response filter.
//
//!  Template works for double, long, std::complex, etc
//!  Taps initialized to zeros.
//! \author Tony Kirke
//! \ingroup templates fir
//! \image html fir.gif
//! \image latex fir.eps
template <class Numeric> class fir_coeff {
 public:
  //! Set tap weights
  void settap(long i, Numeric tap) { coeff[i] = tap; }
  Numeric gettap(long i) { return (coeff[i]); }
  //! Get sum of coefficients
  Numeric coeff_sum() {
    int i;
    Numeric s(0);
    for (i = 0; i < num_taps; i++) s += coeff[i];
    return (s);
  }
  //! Constructor
  ~fir_coeff(void) {}
  //! Constructor
  fir_coeff(void) { ; }
  //! Constructor
  fir_coeff(long n) : coeff(n), num_taps(n) { set_size(n); }
  //! Set size of Filter
  void set_size(long n) {
    int i;
    num_taps = n;
    if (n > 0) {
      coeff.resize(n);
      for (i = 0; i < n; i++) coeff[i] = (Numeric)0;
    }
  }
  long get_size(void) { return (num_taps); }
  //!  Constructor that gets coefficients from file (requires fir_coeff.cpp)
  fir_coeff(const char* file) { read_taps(file); }
  int read_taps(const char* file) { return (0); }
  void make_hpf() {
    bool inv = true;
    for (int i = 0; i < num_taps; i++) {
      if (inv) coeff[i] *= -1;
      inv = !inv;
    }
  }
  void print(void) {
    std::cout << "coeff[] = ";
    for (int i = 0; i < num_taps; i++) std::cout << coeff[i] << ",";
    std::cout << "\n";
  }
  template <class N> friend std::vector<N> get_taps(fir_coeff<N> x);
  void settap(std::vector<Numeric> z) {
    for (int i = 0; i < num_taps; i++) coeff[i] = z[i];
  }
  // Get frequency response at freq
  float_type freqz_mag(float_type freq) {
    int i;
    std::complex<float_type> z(1, 0);
    std::complex<float_type> z_inc = std::complex<float_type>(cos(freq), sin(freq));
    std::complex<float_type> nom(0);
    for (i = 0; i < num_taps; i++) {
      nom += z * (std::complex<float_type>(coeff[i]));
      z *= z_inc;
    }
    return sqrt(std::norm(nom));
  }

 private:
  std::vector<Numeric> coeff;
  long num_taps;
};

template <class T> std::vector<T> get_taps(fir_coeff<T> f) {
  long N = f.num_taps;
  std::vector<T> V(N);
  for (int i = 0; i < N; i++) V[i] = f.coeff[i];
  return (V);
}
}  // namespace spuce
