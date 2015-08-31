########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_BLOCKS_SERIALIZE "Enable Pothos Blocks.Serialize component" ON "ENABLE_BLOCKS" OFF)
add_feature_info("  Serialize" ENABLE_BLOCKS_SERIALIZE "Serialize data to/from byte stream")
if (NOT ENABLE_BLOCKS_SERIALIZE)
    return()
endif()

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
