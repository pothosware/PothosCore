########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_BLOCKS_FILE "Enable Pothos Blocks.File component" ON "ENABLE_BLOCKS" OFF)
add_feature_info("  File" ENABLE_BLOCKS_FILE "Filesystem resource source/sink blocks")
if (NOT ENABLE_BLOCKS_FILE)
    return()
endif()

########################################################################
# File blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET FileBlocks
    SOURCES
        BinaryFileSource.cpp
        BinaryFileSink.cpp
        TestBinaryFileBlocks.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
