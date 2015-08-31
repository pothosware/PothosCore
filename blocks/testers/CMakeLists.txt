########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_BLOCKS_TESTERS "Enable Pothos Blocks.Testers component" ON "ENABLE_BLOCKS" OFF)
add_feature_info("  Testers" ENABLE_BLOCKS_TESTERS "Unit test verification support blocks")
if (NOT ENABLE_BLOCKS_TESTERS)
    return()
endif()

########################################################################
# Tester blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET TesterBlocks
    SOURCES
        TestUnitTestBlocks.cpp
        TestProxyTopology.cpp
        TestJSONTopology.cpp
        TestSetThreadPool.cpp
        FeederSource.cpp
        CollectorSink.cpp
        BlackHole.cpp
        FiniteRelease.cpp
        InfiniteSource.cpp
        SporadicDropper.cpp
        SporadicLabeler.cpp
        VectorSource.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
