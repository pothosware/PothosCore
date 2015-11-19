#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <iostream>
#include <fstream>
#include <spuce/typedefs.h>
#include <spuce/mixed_type.h>
namespace spuce {
//! \file
//! \brief  Template Class for shelf filter
//!   Template for shelving filter
//!   <p>The filter is assumed of the form
//!   \f$ H(s) = \frac{s+A}{s+B} \f$
//!   which becomes
//!   \f$ H(z) =  \frac{(A+1)+(A-1)*z^-1}{(B+1)+(B-1)*z^-1} \f$
//! \author Tony Kirke
//! \ingroup double_templates iir
template <class Numeric, class Coeff = float_type> class iir_shelf {
 protected:
  Coeff b;
  Coeff a0;
  Coeff a1;
  Numeric out;
  Numeric previous_out;
  Numeric previous_in;

 public:
  // Constructor specifying coefficients
  iir_shelf(Coeff A0, Coeff A1, Coeff B) : b(B), a0(A0), a1(A1) { reset(); }
  // Constructor specifying lower & upper frequencies rather than coefficients
  iir_shelf(float_type ca, float_type cb)
      : b((1.0 - cb) / (1.0 + cb)), a0((ca + 1.0) / (cb + 1.0)), a1((1.0 - ca) / (1.0 + b)) {
    reset();
  }
  void set(float_type ca, float_type cb) {
    a0 = ((ca + 1.0) / (cb + 1.0));
    a1 = ((1.0 - ca) / (1.0 + b));
    b = ((1.0 - cb) / (1.0 + cb));
    reset();
  }
  Coeff get_a0() { return (a0); }
  Coeff get_a1() { return (a1); }
  Coeff get_b() { return (b); }
  //! Input new sample and calculate output
  Numeric clock(Numeric input) {
    typename mixed_type<Numeric, Coeff>::dtype sum;
    // Shift previous outputs and calculate new output */
    sum = b * previous_out + a0 * input - a1 * previous_in;
    out = (sum);
    previous_out = out;
    previous_in = input;
    return (out);
  }
  //! Reset
  void reset() { previous_in = previous_out = out = (Numeric)0; }
};
}  // namespace spuce
