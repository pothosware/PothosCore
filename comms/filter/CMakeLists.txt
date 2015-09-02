########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_FILTER "Enable Pothos Comms.Filter component" ON "ENABLE_COMMS" OFF)
add_feature_info("  Filter" ENABLE_COMMS_FILTER "Digital filter and tap designer blocks")
if (NOT ENABLE_COMMS_FILTER)
    return()
endif()

########################################################################
# Filter blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET FilterBlocks
    SOURCES
        DCRemoval.cpp
        FIRDesigner.cpp
        FIRFilter.cpp
        TestFIRFilter.cpp
        EnvelopeDetector.cpp
        WindowFunction.cpp
    DESTINATION comms
    ENABLE_DOCS
)
