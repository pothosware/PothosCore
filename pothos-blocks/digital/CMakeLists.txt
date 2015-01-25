########################################################################
# Utility blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET DigitalBlocks
    SOURCES
        TestDifferentialCoding.cpp
        DifferentialEncoder.cpp
        DifferentialDecoder.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
