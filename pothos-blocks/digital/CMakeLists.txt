########################################################################
# Digital blocks module
########################################################################
if(MSVC)
    add_definitions(/bigobj) #arithmetic too large in debug mode
endif(MSVC)

POTHOS_MODULE_UTIL(
    TARGET DigitalBlocks
    SOURCES
        SymbolMapper.cpp
        SymbolSlicer.cpp
        TestSymbolMapperSlicer.cpp
        BytesToSymbols.cpp
        TestBytesToSymbols.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
