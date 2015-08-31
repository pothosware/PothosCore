########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_UTILITY "Enable Pothos Comms.Utility component" ON "ENABLE_COMMS" OFF)
add_feature_info("  Utility" ENABLE_COMMS_UTILITY "Miscellaneous utilities for sample streams")
if (NOT ENABLE_COMMS_UTILITY)
    return()
endif()

########################################################################
# Utility blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET UtilityBlocks
    SOURCES
        SignalProbe.cpp
        Threshold.cpp
        WaveTrigger.cpp
    DESTINATION comms
    ENABLE_DOCS
)
