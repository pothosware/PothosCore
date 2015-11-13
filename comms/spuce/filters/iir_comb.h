#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/typedefs.h>
#include <spuce/mixed_type.h>
#include <spuce/dsp_classes/delay.h>
namespace spuce {
//! \file
//! \brief  Template for IIR comb type filter with programmable delay and gain
//! \author Tony Kirke
//! \ingroup double_templates iir
template <class Numeric, class Coeff = float_type> class iir_comb {
 protected:
  Coeff acc_gain;
  Coeff in_gain;
  Numeric out;
  delay<Numeric> dly;
  long delay_size;
  Numeric previous_out;
  Numeric previous_in;

 public:
  iir_comb(Coeff A = 0, long delay = 2) : acc_gain(A), in_gain(1 - A), delay_size(delay) {
    dly.set_size(delay);
    previous_in = previous_out = out = (Numeric)0;
  }
  void init(Coeff A, long delay) {
    in_gain = 1 - A;
    acc_gain = A;
    delay_size = delay;
    dly.set_size(delay);
  }

  void set_coeff(Coeff A) {
    in_gain = 1 - A;
    acc_gain = A;
  }
  //! Input new sample and calculate output
  Numeric clock(Numeric input) {
    typename mixed_type<Numeric, Coeff>::dtype sum;
    // Shift previous outputs and calculate new output
    sum = acc_gain * previous_out + in_gain * input;
    out = (sum);
    previous_out = dly.input(out);
    return (out);
  }
  //! Reset
  void reset() {
    previous_in = previous_out = out = (Numeric)0;
    dly.reset();
  }
};
}  // namespace spuce
