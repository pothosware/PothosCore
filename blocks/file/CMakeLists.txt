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
