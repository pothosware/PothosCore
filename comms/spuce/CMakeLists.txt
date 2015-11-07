########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8)
project(spuce)
enable_language(CXX)

# Add path to spuce to include path
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")

SET(FILT_SRCS  
	filters/butterworth_iir.cpp
	filters/chebyshev_iir.cpp
	filters/chebyshev2_iir.cpp
	filters/elliptic_iir.cpp
	filters/find_roots.cpp
	filters/fir_coeff.cpp
	filters/iir_coeff.cpp
	# FIR stuff
	filters/gaussian_fir.cpp
	filters/butterworth_fir.cpp
	filters/raised_cosine_imp.cpp
	filters/root_raised_cosine_imp.cpp
	filters/remez_fir.cpp
	filters/sinc_fir.cpp
	filters/remez_fir.cpp
	filters/fir_inv_dft.cpp
	filters/transform_fir.cpp
	# Window
	filters/window.cpp
)

add_library(spucelib STATIC ${FILT_SRCS})
set_property(TARGET spucelib PROPERTY POSITION_INDEPENDENT_CODE TRUE)
