#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <iostream>
#include <spuce/typedefs.h>
#include <spuce/mixed_type.h>
namespace spuce {
//! \file
//! \brief Template for 2nd order allpass filter<p>
//! \author Tony Kirke
//! \ingroup double_templates Allpass
//!  Template for 2nd order allpass filter<p>
//! The filter is assumed to be a  2nd order allpass section of the form
//! \f$ G(z) =  \frac{z^2 + b*z+a}{(a*z^2+b*z+1)} \f$
template <class Numeric, class Coeff = float_type> class allpass_2nd {
 protected:  // should be protected:
  Coeff b;
  Coeff a;
  Numeric in[3];

 public:
  Numeric fb2, fb1, fb0;
  typedef typename mixed_type<Numeric, Coeff>::dtype sum_type;

 public:
  allpass_2nd(Coeff B0, Coeff A0, long b = 0) : b(B0), a(A0) {
    reset();
  }
  allpass_2nd() : b(1), a(1) {
    reset();
  }
  void reset() { in[0] = in[1] = in[2] = fb2 = fb1 = fb0 = (Numeric)0; }
  void set_a(Coeff A0) { a = A0; }
  void set_b(Coeff B0) { b = B0; }
  Coeff get_a() { return (a); }
  Coeff get_b() { return (b); }
  void set_coeffs(Coeff B0, Coeff A0) {
    b = B0;
    a = A0;
  }
  //! Print out coefficients
  void print() {
    std::cout << "Allpass Coefficients B = " << b;
    std::cout << ", A =  " << a;
    std::cout << ".\n";
  }
  //! Input new sample and calculate output
  Numeric clock(Numeric input) {
    sum_type sum;
    // Shift inputs by one time sample and place new sample into array
    in[0] = in[1];
    in[1] = in[2];
    in[2] = input;
    // Shift previous outputs and calculate new output */
    fb0 = fb1;
    fb1 = fb2;

    sum = (a * (in[1] - fb1) + b * (in[2] - fb0) + in[0]);

    fb2 = (sum);
    return ((sum));
  }
};
}  // namespace spuce
