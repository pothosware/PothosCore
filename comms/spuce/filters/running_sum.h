#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/dsp_classes/delay.h>
namespace spuce {
//! \file
//! \brief template class running average filter consisting of a delay line, adder and subtractor
//! \author Tony Kirke
//! \ingroup templates fir
//! \image html running_sum.gif
//! \image latex running_sum.eps
template <class Numeric> class running_sum {
 protected:
  Numeric result;
  delay<Numeric> z;
  long size;
  float_type inv_size;

 public:
  //!/ Constructor
  running_sum(long n = 0) : z() {
    size = n;
    inv_size = 1.0 / (float_type)size;
    if (n > 1) {
      z.set_size(n - 1);
      z.reset();
    }
    result = (Numeric)0;
  }
  //! Assignment
  running_sum& operator=(const running_sum& rhs) {
    z = rhs.z;
    result = rhs.result;
    return (*this);
  }
  //!
  void set_size(long n) {
    z.set_size(n - 1);
    result = (Numeric)0;
    size = n;
    inv_size = 1.0 / (float_type)size;
  }
  //! destructor
  ~running_sum(void) {}
  //! Reset/clear
  void reset(void) {
    z.reset();
    result = (Numeric)0;
  }
  //! return result
  Numeric get_result() { return (result); }
  //! Clock in new input sample
  Numeric update(Numeric in) {
    result -= z.last();
    z.input(in);
    result += in;
    return (result);
  }
  Numeric average(Numeric in) { return (((Numeric)inv_size * update(in))); }
};
}  // namespace spuce
