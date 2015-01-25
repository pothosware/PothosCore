########################################################################
# Utility blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET DigitalBlocks
    SOURCES
        TestDifferentialCoding.cpp
        DifferentialEncoder.cpp
        DifferentialDecoder.cpp
        SymbolsToBits.cpp
        BitsToSymbols.cpp
        TestSymbolBitConversions.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
