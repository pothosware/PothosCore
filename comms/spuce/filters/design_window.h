#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/filters/window.h>
namespace spuce {
//! \file
//! \brief Design functions for window functions
//! \author Tony Kirke
//! \ingroup functions fir
std::vector<float_type> design_window(const std::string& fir_type, int order, float_type beta=4);
}  // namespace spuce
