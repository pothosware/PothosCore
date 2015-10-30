#pragma once
// Copyright (c) 2015 Tony Kirke.  Boost Software License - Version 1.0  (http://www.opensource.org/licenses/BSL-1.0)

// Putting std::complex here allows possible replacement with custom complex type later
#include <complex>
#include "complex_operators.h"

namespace spuce {
	// if defined USE_FLOAT, default to float type
#ifndef USE_FLOAT
	typedef double float_type;
#else
	typedef float float_type;
#endif

}  // namespace spuce
