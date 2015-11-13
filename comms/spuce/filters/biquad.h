#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <iostream>
#include <fstream>
#include <spuce/typedefs.h>
#include <spuce/mixed_type.h>
namespace spuce {
//! \file
//! \brief Template for 2nd Order IIR filter<p>
//!   The filter is assumed to be a low pass 2nd order digital filter
//!   of the form \f$ G(z) =  \frac{b0*z^2 + b1*z+b2}{(z^2+a1*z+a2)} \f$
//! \image html iir_2nd.gif
//! \image latex iir_2nd.eps
//! \author Tony Kirke
//! \ingroup double_templates iir
template <class Numeric, class Coeff = float_type> class biquad {
 protected:  // should be protected:
  Coeff b0, b1, b2;
  Coeff a1, a2;
  Numeric in[3];
 public:
  Numeric fb2, fb1, fb0;
  //  Numeric scale;
  typedef typename mixed_type<Numeric, Coeff>::dtype sum_type;

 public:
  biquad(Coeff B0, Coeff B1, Coeff B2, Coeff A1, Coeff A2) : b0(B0), b1(B1), b2(B2), a1(A1), a2(A2) {
    reset();
    fb0 = fb1 = fb2 = (Numeric)0;
  }
  biquad(Coeff A1 = 0, Coeff A2 = 0) : b0(1), b1(2), b2(1), a1(A1), a2(A2) {
    reset();
    fb0 = fb1 = fb2 = (Numeric)0;
    //	scale = 1;
  }
  Coeff get_b0() { return (b0); }
  Coeff get_b1() { return (b1); }
  Coeff get_b2() { return (b2); }
  Coeff get_a1() { return (a1); }
  Coeff get_a2() { return (a2); }

  void reset() { in[0] = in[1] = in[2] = fb2 = fb1 = fb0 = (Numeric)0; }
  void set_a(Coeff A1, Coeff A2) {
    a1 = A1;
    a2 = A2;
  }
  void set_b(Coeff A1, Coeff A2) {
    b1 = A1;
    b2 = A2;
  }
  void set_coeffs(Coeff B0, Coeff B1, Coeff B2, Coeff A1, Coeff A2) {
    b0 = B0;
    b1 = B1;
    b2 = B2;
    a1 = A1;
    a2 = A2;
  }
  //! Print out coefficients
  void print() {
    std::cout << "IIR Coefficients B0 = " << b0;
    std::cout << ", B1 =  " << b1;
    std::cout << ", B2 =  " << b2;
    std::cout << ", A0 =  1";
    std::cout << ", A1 =  " << a1;
    std::cout << ", A2 =  " << a2;
    std::cout << ".\n";
  }
  //! Input new sample and calculate output
  Numeric clock(Numeric input) {
    sum_type sum;
    // Shift inputs by one time sample and place new sample into array
    in[0] = in[1];
    in[1] = in[2];
    in[2] = input;
    // Shift previous outputs and calculate new output
    fb0 = fb1;
    fb1 = fb2;

    sum = (b0 * in[2] + b1 * in[1] + b2 * in[0] - a1 * fb1 - a2 * fb0);

    fb2 = sum;
    return (fb2);
  }
};
}  // namespace spuce
