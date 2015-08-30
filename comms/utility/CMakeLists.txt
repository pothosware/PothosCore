########################################################################
# Utility blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET UtilityBlocks
    SOURCES
        SignalProbe.cpp
        Threshold.cpp
        WaveTrigger.cpp
    DESTINATION comms
    ENABLE_DOCS
)
