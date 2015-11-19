########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_FILTER "Enable Pothos Comms.Filter component" ON "ENABLE_COMMS" OFF)
add_feature_info("  Filter" ENABLE_COMMS_FILTER "Digital filter and tap designer blocks")
if (NOT ENABLE_COMMS_FILTER)
    return()
endif()

include_directories("${PROJECT_SOURCE_DIR}")

########################################################################
# Filter blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET FilterBlocks
    SOURCES
        DCRemoval.cpp
        FIRDesigner.cpp
        TestFIRDesigner.cpp
        IIRDesigner.cpp
        FIRFilter.cpp
	IIRFilter.cpp
        TestFIRFilter.cpp
        TestIIRFilter.cpp
        EnvelopeDetector.cpp
	DESTINATION comms
    LIBRARIES spucelib
    ENABLE_DOCS
)
