########################################################################
# Filter blocks module
########################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fir_filter/include)
POTHOS_MODULE_UTIL(
    TARGET FilterBlocks
    SOURCES
        FIRDesigner.cpp
        FIRFilter.cpp
        TestFIRFilter.cpp
        fir_filter/src/filt.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
