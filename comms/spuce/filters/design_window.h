#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)
#include <spuce/filters/window.h>
namespace spuce {
//! \file
//! \brief Design functions for window functions
//! \author Tony Kirke
//! \ingroup functions fir
std::vector<float_type> design_window(const std::string& fir_type, int order, float_type beta=4) {

	std::vector<float_type> win;
	if (fir_type == "hamming") {
		win = hamming(order);
	} else if (fir_type == "hanning") {
		win = hanning(order);
	} else if (fir_type == "hann") {
		win = hann(order);
	} else if (fir_type == "blackman") {
		win =  blackman(order);
	} else if (fir_type == "kaiser") {
		win =  kaiser(order, beta);
	} else if (fir_type == "bartlett") {
		win = bartlett(order);
	} else if (fir_type == "chebyshev") {
		win = cheby(order, beta);
	} else {
		std::cout << "Invalid type " << fir_type << "\n";
	}

	// Normalize DC response to 1.0
	float_type sum = 0;
	for (size_t i=0;i<win.size();i++) sum += win[i];
	for (size_t i=0;i<win.size();i++) win[i] /= sum;
	return win;
}
}  // namespace spuce
