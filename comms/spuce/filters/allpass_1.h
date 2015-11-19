#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <iostream>
#include <spuce/typedefs.h>
#include <spuce/mixed_type.h>
#include <spuce/dsp_classes/circ_buffer.h>
namespace spuce {
//! \file
//! \brief  Template class for 1st order allpass filter with programmble gain and delay
//! Template class for 1st-order Allpass filter.
//! The filter is a "first-order" Allpass section of the form  G(z,n) =  (a*z^n + 1)/(z^n+a)
//! The delay n can be specified
//! \image html allpass_1.gif
//! \image latex allpass_1.eps
//! \author Tony Kirke
//! \ingroup double_templates Allpass
template <class Numeric, class Coeff = float_type> class allpass_1 {
 protected:
  //! number of unit sample delays
  long size;
  float_type gain;
  circ_buffer<Numeric> mult;
  circ_buffer<Numeric> sum;
  typedef typename mixed_type<Numeric, Coeff>::dtype sum_type;

 public:
  //! Constructor with gain and delay
 allpass_1(Coeff Gain = 0, long s = 1) {
    gain = Gain;
    size = s;
    mult.set_size(size);
    sum.set_size(size);
  }
  Coeff get_coefficient(void) { return ((Coeff)(gain)); }
  //! Reset
  void reset() {
    mult.reset();
    sum.reset();
  }
  void set_gain(Coeff Gain) { gain = Gain; }
  //! Initializes gain and delay
  void init(const Coeff& Gain, long s) {
    gain = Gain;
    size = s;
    mult.set_size(size);
    sum.set_size(size);
  }
  //! Shift inputs by one time sample and place new sample into array
  Numeric clock(Numeric input) {
    Numeric current_mult, out;
    Numeric current_sum;
    sum_type mult_gain;

    out = sum.check(size - 1);
    current_sum = (input + mult.check(size - 1));
    mult_gain = -gain * current_sum;

    current_mult = (mult_gain);
    out -= current_mult;
    // update delay lines
    mult.input(current_mult);
    sum.input(current_sum);
    return (out);
  }
};
}  // namespace spuce
