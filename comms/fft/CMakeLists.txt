########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_FFT "Enable Pothos Comms.FFT component" ON "ENABLE_COMMS" OFF)
add_feature_info("  FFT" ENABLE_COMMS_FFT "Implementation of the fast fourier transform")
if (NOT ENABLE_COMMS_FFT)
    return()
endif()

########################################################################
# Filter blocks module
########################################################################

#The templated c++ implementation only handles floats.
#We use the C implementation to handle the fixed point,
#however only one integer type can be compiled at once.
#We assume that complex int16 will be the most useful.

#add_definitions(-DFIXED_POINT=32)
add_definitions(-DFIXED_POINT=16)

include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(alloca.h HAS_ALLOCA_H)
if(HAS_ALLOCA_H)
    add_definitions(-DHAS_ALLOCA_H)
endif(HAS_ALLOCA_H)

POTHOS_MODULE_UTIL(
    TARGET FFTBlocks
    SOURCES
        FFT.cpp
        kiss_fft.c
        TestFFT.cpp
    DESTINATION comms
    ENABLE_DOCS
)
