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
