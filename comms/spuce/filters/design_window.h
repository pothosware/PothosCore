#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/filters/window.h>
namespace spuce {
//! \file
//! \brief Design functions for window functions
//! \author Tony Kirke
//! \ingroup functions fir
std::vector<float_type> design_window(const std::string& fir_type, int order, float_type tw=0.1, float_type ripple_db=0.1) {

	std::vector<float_type> win;
	if (fir_type == "hamming") {
		win = hamming(order, 0.54, 0.46);
	}	else if (fir_type == "hanning") {
		win = hanning(order);
	}	else if (fir_type == "blackman") {
		win =  blackman(order);
	}	else if (fir_type == "kaiser") {
		win =  kaiser(order, tw, ripple_db);
  } else {
		win = bartlett(order);
  }

	// Normalize DC response to 1.0
	float_type sum = 0;
	for (size_t i=0;i<win.size();i++) sum += win[i];
	for (size_t i=0;i<win.size();i++) win[i] /= sum;
	return win;
}
}  // namespace spuce
