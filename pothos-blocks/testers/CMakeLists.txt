########################################################################
# Tester blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET TesterBlocks
    SOURCES
        TestUnitTestBlocks.cpp
        TestProxyTopology.cpp
        TestSetThreadPool.cpp
        FeederSource.cpp
        CollectorSink.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
