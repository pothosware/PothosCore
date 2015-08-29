########################################################################
# Utility blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET UtilityBlocks
    SOURCES
        Delay.cpp
        TestDelay.cpp
        StreamProbe.cpp
        Threshold.cpp
        WaveTrigger.cpp
    DESTINATION comms
    ENABLE_DOCS
)
