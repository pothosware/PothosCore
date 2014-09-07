########################################################################
# Math blocks module
########################################################################
if(MSVC)
    add_definitions(/bigobj) #arithmetic too large in debug mode
endif(MSVC)

POTHOS_MODULE_UTIL(
    TARGET MathBlocks
    SOURCES
        Arithmetic.cpp
        TestArithmeticBlocks.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
