########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_COMMS_MAC "Enable Pothos Comms.MAC component" ON "ENABLE_COMMS" OFF)
add_feature_info("  MAC" ENABLE_COMMS_MAC "MAC layer and packet protocol blocks")
if (NOT ENABLE_COMMS_MAC)
    return()
endif()

########################################################################
# MAC blocks module
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
