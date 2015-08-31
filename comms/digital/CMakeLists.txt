########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_DIGITAL "Enable Pothos Comms.Digital component" ON "ENABLE_COMMS" OFF)
add_feature_info("  Digital" ENABLE_COMMS_DIGITAL "Digital byte, bit, and symbol operators")
if (NOT ENABLE_COMMS_DIGITAL)
    return()
endif()

########################################################################
# Digital blocks module
########################################################################
if(MSVC)
    add_definitions(/bigobj) #may be helpful for templated factories
endif(MSVC)

POTHOS_MODULE_UTIL(
    TARGET DigitalBlocks
    SOURCES
        SymbolMapper.cpp
        SymbolSlicer.cpp
        TestSymbolMapperSlicer.cpp
        BytesToSymbols.cpp
        SymbolsToBytes.cpp
        TestDifferentialCoding.cpp
        DifferentialEncoder.cpp
        DifferentialDecoder.cpp
        SymbolsToBits.cpp
        BitsToSymbols.cpp
        TestSymbolBitConversions.cpp
        TestSymbolByteConversions.cpp
        PreambleCorrelator.cpp
        PreambleFramer.cpp
        TestFramerToCorrelator.cpp
        TestPreambleFramer.cpp
        TestPreambleCorrelator.cpp
        Scrambler.cpp
        Descrambler.cpp
        FrameInsert.cpp
        FrameSync.cpp
    DESTINATION comms
    ENABLE_DOCS
)
