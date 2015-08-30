########################################################################
# Packet blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET PacketBlocks
    SOURCES
        SimpleMac.cpp
        TestSimpleMac.cpp
        SimpleLlc.cpp
        TestSimpleLlc.cpp
        PacketToStream.cpp
        StreamToPacket.cpp
        TestPacketBlocks.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
