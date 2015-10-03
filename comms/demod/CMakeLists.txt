########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_DEMOD "Enable Pothos Comms.Demod component" ON "ENABLE_COMMS" OFF)
add_feature_info("  Demod" ENABLE_COMMS_DEMOD "Basic communications demodulation blocks")
if (NOT ENABLE_COMMS_DEMOD)
    return()
endif()

########################################################################
# Demod blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET DemodBlocks
    SOURCES
        FreqDemod.cpp
    DESTINATION comms
    ENABLE_DOCS
)
