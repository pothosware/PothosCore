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
POTHOS_MODULE_UTIL(
    TARGET FFTBlocks
    SOURCES
        FFT.cpp
        TestFFT.cpp
        #FFTImplFloat32.cpp
        #FFTImplInt16.cpp
    DESTINATION comms
    ENABLE_DOCS
)
