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
        TestBytesToSymbols.cpp
        TestDifferentialCoding.cpp
        DifferentialEncoder.cpp
        DifferentialDecoder.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
