#pragma once
// Copyright (c) 2015 Tony Kirke. License MIT  (http://www.opensource.org/licenses/mit-license.php)

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
