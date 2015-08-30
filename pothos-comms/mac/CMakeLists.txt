########################################################################
# Packet blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET MacBlocks
    SOURCES
        SimpleMac.cpp
        TestSimpleMac.cpp
        SimpleLlc.cpp
        TestSimpleLlc.cpp
    DESTINATION comms
    ENABLE_DOCS
)
