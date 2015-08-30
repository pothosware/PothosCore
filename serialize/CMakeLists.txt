########################################################################
# Serialize blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET SerializeBlocks
    SOURCES
        Serializer.cpp
        Deserializer.cpp
        TestSerialize.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
