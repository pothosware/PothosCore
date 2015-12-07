########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_BLOCKS_STREAM "Enable Pothos Blocks.Stream component" ON "ENABLE_BLOCKS" OFF)
add_feature_info("  Stream" ENABLE_BLOCKS_STREAM "Miscellaneous stream manipulation blocks")
if (NOT ENABLE_BLOCKS_STREAM)
    return()
endif()

########################################################################
# Stream blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET StreamBlocks
    SOURCES
        Converter.cpp
        TestConverter.cpp
        Copier.cpp
        Delay.cpp
        TestDelay.cpp
        DynamicRouter.cpp
        Pacer.cpp
        Relabeler.cpp
        Gateway.cpp
        TestGateway.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
