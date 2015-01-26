########################################################################
# Filter blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET FilterBlocks
    SOURCES
        FIRDesigner.cpp
        FIRFilter.cpp
        TestFIRFilter.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
