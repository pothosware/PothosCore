#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/typedefs.h>
#include <spuce/filters/iir_1st.h>
#include <spuce/filters/biquad.h>
#include <spuce/filters/iir_coeff.h>
#include <iostream>
#include <vector>
namespace spuce {
//! \file
//! \brief Tempate Class for iir filter consisting of first and second order filter sections
//! \author Tony Kirke
//! \ingroup double_templates iir
template <class Numeric, class Coeff = float_type> class iir {
 private:
  long order;
  long odd;
  long n2;
  bool lpf;
  std::vector<biquad<Numeric, Coeff>> sos;
  iir_1st<Numeric, Coeff> fos;
  Coeff gain;
  typedef typename mixed_type<Numeric, Coeff>::dtype mult_type;

 public:
  //! ord = Filter order
  iir(iir_coeff& design) : sos(design.getN2()), fos(1) {
		order = design.getOrder();
    n2 = design.getN2();
    odd = design.isOdd();
		lpf = (design.get_type() == filter_type::low);
    set_coeffs(design);
  }
  //! ord = Filter order
  iir(long ord = 1) : sos((ord + 1) / 2), fos(1) {
    // amax - attenuation at cutoff
    lpf = true;
    order = ord;
    n2 = (order + 1) / 2;
    odd = (order % 2);
  }
  void clear() { sos.resize(0); }
  void realloc(iir_coeff& design) {
		lpf = (design.get_type() == filter_type::low);
    sos.resize(design.getN2());
    order = design.getOrder();
    n2 = design.getN2();
    odd = design.isOdd();
    set_coeffs(design);
    reset();
  }

  //! Destructor
  ~iir() {}
  int stages() { return (n2); }
  //! Reset history
  void reset() {
    for (int j = odd; j < n2; j++) sos[j - odd].reset();
    if (odd) fos.reset();
  }
  //! Set 2nd order IIR coefficients
  void set_coeffs(iir_coeff& design) {
    bool ok = design.calculate_biquad_coefficents();
    if (ok) {
      if (odd) {
        if (lpf)
          fos.set_coeffs(-real(design.get_pole(0)), 1.0);
        else
          fos.set_coeffs(-real(design.get_pole(0)), -1.0);
      }
      for (int j = odd; j < n2; j++) {
        sos[j - odd].set_a(real(design.get_pole(j)), imag(design.get_pole(j)));
        sos[j - odd].set_b(real(design.get_zero(j)), imag(design.get_zero(j)));
      }
      gain = (Coeff)design.getGain();
    } else {
      std::cout << "Error can not set coefficients";
    }
  }
  //! print coefficients
  void print() {
    if (odd) fos.print();
    for (int j = odd; j < n2; j++) sos[j - odd].print();
  }
  //! Clock in sample and get output.
  Numeric clock(Numeric in) {
    Numeric tmp = in;
    for (int i = odd; i < n2; i++) { tmp = sos[i - odd].clock(tmp); }
    if (odd) { tmp = fos.clock(tmp); }
    mult_type g = gain * tmp;
    return (g);
  }
  void process(const std::vector<Numeric>& in, std::vector<Numeric>& out) {
    for (int j = 0; j < in.size(); j++) out[j] = clock(in[j]);
  }
  void process_inplace(std::vector<Numeric>& io) {
    for (int j = 0; j < io.size(); j++) io[j] = clock(io[j]);
  }
};
}  // namespace spuce
