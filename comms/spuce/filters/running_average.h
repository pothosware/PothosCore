#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/dsp_classes/delay.h>
namespace spuce {
//! \file
//! \brief template class running average filter consisting of a delay line, adder and subtractor
//
//! \ingroup templates templates fir
//! \image html running_average.png
//! \image html running_average.gif
//! \image latex running_average.eps
//! \author Tony Kirke
template <class Numeric> class running_average {
 protected:
  Numeric result;
  delay<Numeric> z;
  long size;
  float_type inv_size;

 public:
  //!/ Constructor
  running_average(long n = 0) : z() {
    size = n;
    inv_size = 1.0 / (float_type)size;
    if (n > 1) {
      z.set_size(n - 1);
      z.reset();
    }
    result = (Numeric)0;
  }
  //! Assignment
  running_average& operator=(const running_average& rhs) {
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
  ~running_average(void) {}
  //! Reset/clear
  void reset(void) {
    z.reset();
    result = (Numeric)0;
  }
  Numeric update(Numeric in) {
    result -= z.last();
    z.input(in);
    result += in;
    return (((Numeric)inv_size * update(in)));
  }
};
}  // namespace spuce
