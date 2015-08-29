########################################################################
# Stream blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET StreamBlocks
    SOURCES
        Converter.cpp
        TestConverter.cpp
        Copier.cpp
        DynamicRouter.cpp
        Pacer.cpp
        Relabeler.cpp
        Gateway.cpp
        TestGateway.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
