########################################################################
# Packet blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET PacketBlocks
    SOURCES
        PacketToStream.cpp
        StreamToPacket.cpp
        TestPacketBlocks.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
