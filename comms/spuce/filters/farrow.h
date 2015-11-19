#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/typedefs.h>
#include <vector>
#include <cassert>
namespace spuce {
//! \file
//! \brief  Template Class for Farrow implementation of a ploynomial interpolation using a FIR  filter.
//
//!   Farrow implementation of a polynomial interpolation
//!   This covers 3 (orders) cases of interpolation
//!   1. Linear (not efficient using this method)
//!   2. Piecewise-Parabolic
//!   3. Cubic
//!   Although derived from the Lagrange coefficients the
//!   multiplication factor have been simplified to allow
//!   simple VLSI implementations. see
//!   Interpolation in Digital Modem - Part II, IEEE Trans.
//!   on Comms. June 1993.
//! \author Tony Kirke
//!  \ingroup templates fir interpolation
template <class Numeric> class farrow {
 public:
  long num_taps;
  std::vector<Numeric> coeff;

 protected:
  std::vector<Numeric> z;
  Numeric output;

 public:
  //! Get current output
  Numeric out() { return (output); }
  //! Clock in new sample & compute current output
  Numeric check(long i) {
    assert(i < num_taps + 1);
    return (z[i]);
  }
  //! Need to cover different orders!
  farrow(char order) : num_taps(order), coeff(num_taps + 1), z(num_taps + 2) {
    int i;
    assert((order > 0) & (order < 5));
    for (i = 0; i <= num_taps; i++) {
      z[i] = (Numeric)0;
      coeff[i] = (Numeric)0;
    }
  }
  //! reset
  void reset(void) {
    int i;
    for (i = 0; i <= num_taps; i++) z[i] = (Numeric)0;
    output = (Numeric)0;
  }
  ~farrow(void) {}
  //! Only input new sample
  void input(Numeric in) {
    int i;
    //! Update history of inputs
    for (i = num_taps + 1; i > 0; i--) z[i] = z[i - 1];
    //! Add new input
    z[0] = in;
  }

  //!:
  //!   Update => Clock in new input sample, recalculate coefficients and
  //!   determine output
  Numeric update(Numeric in, float_type offset) {
    int i;
    //! Update history of inputs
    for (i = num_taps + 1; i > 0; i--) z[i] = z[i - 1];
    //! Add new input
    z[0] = in;
    //! Calculate coefficients
    if (num_taps == 1)
      return (in);
    else if (num_taps == 2)
      calculate_coeff2();
    else if (num_taps == 3)
      calculate_coeff3a();
    else if (num_taps == 4)
      calculate_coeff4();
    //! Perform FIR
    return (fir(offset));
  }
  //! Calculate output for current coefficients. Offset is between 0 and 1
  Numeric fir(float_type offset) {
    //! Perform FIR
    float_type gain = offset;
    output = coeff[0];
    for (int i = 1; i < num_taps; i++) {
      output += (Numeric)(offset * coeff[i]);
      offset *= gain;
    }
    return (output);
  }
  //! Rephase => recalculate coefficients and output for new offset (for upsampling)
  Numeric rephase(float_type offset) {
    if (num_taps == 1)
      return (z[0]);
    else if (num_taps == 2)
      calculate_coeff2();
    else if (num_taps == 3)
      calculate_coeff3a();
    else if (num_taps == 4)
      calculate_coeff4();
    return (fir(offset));
  }
  //! Calculate coefficients for linear (not tested)
  void calculate_coeff2(void) {
    //! Calculate coefficients
    coeff[1] = z[0] - z[1];
    coeff[0] = z[1];
  }
  //! Calculate coefficients for square
  void calculate_coeff3(void) {
    //! Calculate coefficients
    coeff[2] = z[0] - ((Numeric)2) * z[1] + z[2];
    coeff[1] = z[0] - z[2];
    coeff[0] = ((Numeric)2) * z[1];
  }
  //! Calculate coefficients for parabolic
  void calculate_coeff3a(void) {
    //! Calculate coefficients
    coeff[2] = (z[0] - z[1] - z[2] + z[3]);
    coeff[1] = -z[0] + ((Numeric)3) * z[1] - z[2] - z[3];
    coeff[0] = ((Numeric)2) * z[2];
  }
  //! Calculate coefficients for 4th order
  void calculate_coeff4(void) {
    coeff[3] = -z[3] + ((Numeric)3) * z[2] - ((Numeric)3) * z[1] + z[0];
    coeff[2] = ((Numeric)3) * z[3] - ((Numeric)6) * z[2] + ((Numeric)3) * z[1];
    coeff[1] = ((Numeric)-2) * z[3] - ((Numeric)3) * z[2] + ((Numeric)6) * z[1] - z[0];
    coeff[0] = ((Numeric)6) * z[2];
  }
};
}  // namespace spuce
