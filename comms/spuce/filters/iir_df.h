#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/filters/iir_coeff.h>
#include <spuce/filters/fir.h>
namespace spuce {
//! \file
//!  \brief Template Class for Modeling a Direct Form IIR
//
//!  Template works for float_type, long, std::complex, etc
//!  Implemented as cascade of FIR part followed by IIR part
//!  IIR function implemented with FIR class using special iir function
//! \author Tony Kirke
//!  \ingroup double_templates iir
template <class Numeric, class Coeff = float_type> class iir_df {
 public:
	//  iir_df<Numeric, Coeff>() { ; }
  iir_df<Numeric, Coeff>(const int order=1) : poles(order), zeros(order + 1) {
    for (auto i = 0; i < order + 1; i++) zeros.settap(i,1.0);
    for (auto i = 0; i < order; i++) poles.settap(i,0.0);
  }

  iir_df<Numeric, Coeff>(const iir_df<Numeric, Coeff>& cp) : poles(a_size()),	zeros(b_size()) {
    for (auto i = 0; i < a_size(); i++) poles.settap(i, cp.poles.get_coeff(i));
    for (auto i = 0; i < b_size(); i++) zeros.settap(i, cp.zeros.get_coeff(i));
  }

  iir_df<Numeric, Coeff>(const iir_coeff& filt) : poles(filt.getOrder()), zeros(filt.getOrder() + 1) {
    // Apply gain to feedforward taps to normalize
    for (auto i = 0; i < filt.getOrder() + 1; i++) zeros.settap(i, filt.get_b(i));
    // For feedback, skip a[0] which is 1 and used negative of remaining coefficients
    // for `iir` part
    for (auto i = 0; i < filt.getOrder(); i++) poles.settap(i, -filt.get_a(i + 1));
  }
  ~iir_df(void) {}
  //! Reset
  void reset() {
    poles.reset();
    zeros.reset();
  }
  void set_taps(const std::vector<double>& taps) {
		// Divide vector in half, 1st half are feedforward, 2nd half feedback
		assert(taps.size() != 0);
		auto size = taps.size()/2;
		zeros.set_size(size);
		poles.set_size(size-1);
    for (auto i = 0; i < size; i++) { zeros.settap(i, taps[i]); }
		// Skip 1st feedback and negate the rest
    for (auto i = 0; i < size-1; i++) { poles.settap(i, -taps[i+size+1]); }
		
		//		print();
		
  }
  int order(void) { return zeros.number_of_taps(); }
  Numeric clock(Numeric in) { return (update(in)); }
  Numeric update(Numeric in) {
    Numeric output = zeros.update(in);
    output = poles.iir(output);
    return (output);
  }
  void print(void) {
    std::cout << "B ";
    zeros.print();
    std::cout << "A ";
    poles.print();
  }
	int a_size() const { return poles.number_of_taps(); }
	int b_size() const { return zeros.number_of_taps(); }

 private:
  fir<Numeric, Coeff> poles;
  fir<Numeric, Coeff> zeros;
};
}  // namespace spuce
