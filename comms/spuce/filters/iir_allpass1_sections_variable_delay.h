#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)
#include <spuce/typedefs.h>
#include <spuce/dsp_classes/circ_buffer.h>
#include <spuce/filters/allpass.h>
#include <spuce/filters/elliptic_allpass.h>
#include <iostream>
namespace spuce {
//! \file
//! \brief  Template Class for iir filter consisting of 1st order allpass sections
//! This is a subband IIR filter with two branches of cascades of 1st order allpass sections
//! \image html iir_allpass1_cascade.png
//! \author Tony Kirke
//! \ingroup double_templates double_templates iir
template <class Numeric, class Coeff = float_type> class iir_allpass1_sections_variable_delay {
 public:
  long stages;
  allpass<Numeric, Coeff> A0;
  allpass<Numeric, Coeff> A1;
  circ_buffer<Numeric> prev_input;
  bool hpf;
  Numeric out0, out1;

 public:
  //! n = Filter stages
  iir_allpass1_sections_variable_delay(float_type fp = 0, int n = 1, int dly = 2) {
    int j = 0;
    int k = 0;
    hpf = false;
    prev_input.set_size(dly / 2);
    stages = n;
    // prev_input = (Numeric)0;
    if (stages > 0) {
      std::vector<float_type> a0(stages);
      std::vector<float_type> a1(stages);
      elliptic_allpass(a0, a1, fp, stages);
      j = (stages + 1) / 2;
      k = stages - j;
      // CONVERT FROM DOUBLE to COEFF
      std::vector<Coeff> a0c(stages);
      std::vector<Coeff> a1c(stages);
      for (int i = 0; i < stages; i++) {
          a0c[i] = (Coeff)a0[i];
          a1c[i] = (Coeff)a1[i];
      }
      A0.init(a0c, j, dly);
      A1.init(a1c, k, dly);
    }
  }

  //! n = Filter stages
  void set_coeffs(float_type fp, int n = 1, int dly = 2) {
    int j = 0;
    int k = 0;
    stages = n;
    prev_input.set_size(dly / 2);
    prev_input.reset();  //= (Numeric)0;
    if (stages > 0) {
      std::vector<float_type> a0(stages);
      std::vector<float_type> a1(stages);
      elliptic_allpass(a0, a1, fp, stages);
      j = (stages + 1) / 2;
      k = stages - j;
      // CONVERT FROM DOUBLE to COEFF
      std::vector<Coeff> a0c(stages);
      std::vector<Coeff> a1c(stages);
      for (int i = 0; i < stages; i++) {
          a0c[i] = (Coeff)a0[i];
          a1c[i] = (Coeff)a1[i];
      }

      A0.init(a0c, j, dly);
      A1.init(a1c, k, dly);
    }
  }

  Coeff get_a0(int i) {
    int j = (stages + 1) / 2;
    if ((i < j) && (i > -1))
      return (A0.ap[i].get_coefficient());
    else
      return (0);
  }
  Coeff get_a1(int i) {
    int j = (stages + 1) / 2;
    if ((i < stages - j) && (i > -1))
      return (A1.ap[i].get_coefficient());
    else
      return (0);
  }
  //! Destructor
  ~iir_allpass1_sections_variable_delay() {}
  //! Reset history
  void reset() {
    A0.reset();
    A1.reset();
    prev_input.reset();
  }
  void set_hpf(bool h) { hpf = h; }
  //! Clock in sample and get output.
  Numeric clock(Numeric input) {
    out0 = A0.clock(input);
    out1 = A1.clock(input);
    prev_input.input(out1);
    out1 = prev_input.last();
    if (hpf) {
      return (0.5*(out0 - out1));
    } else {
      return (0.5*(out0 + out1));
    }
  }
  //! Clock in sample and get output.
  Numeric get_hp_out() { return (round((out0 - out1), 1)); }
};
// template_instantiations: float_type

}  // namespace spuce
