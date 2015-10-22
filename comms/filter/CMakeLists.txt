########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_FILTER "Enable Pothos Comms.Filter component" ON "ENABLE_COMMS" OFF)
add_feature_info("  Filter" ENABLE_COMMS_FILTER "Digital filter and tap designer blocks")
if (NOT ENABLE_COMMS_FILTER)
    return()
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")


########################################################################
# Filter blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET FilterBlocks
    SOURCES
        DCRemoval.cpp
        FIRDesigner.cpp
        IIRDesigner.cpp
        FIRFilter.cpp
				IIRFilter.cpp
        TestFIRFilter.cpp
        TestIIRFilter.cpp
        EnvelopeDetector.cpp
        WindowFunction.cpp
		DESTINATION comms
		LIBRARIES spucelib
    ENABLE_DOCS
)
