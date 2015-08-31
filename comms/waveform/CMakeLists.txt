########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_WAVEFORM "Enable Pothos Comms.Waveform component" ON "ENABLE_COMMS" OFF)
add_feature_info("  Waveform" ENABLE_COMMS_WAVEFORM "Periodic and random waveform generation")
if (NOT ENABLE_COMMS_WAVEFORM)
    return()
endif()

########################################################################
# Waveform blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET WaveformBlocks
    SOURCES
        WaveformSource.cpp
        NoiseSource.cpp
    DESTINATION comms
    ENABLE_DOCS
)
