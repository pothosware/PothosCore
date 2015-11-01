########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_BLOCKS_EVENT "Enable Pothos Blocks.Event component" ON "ENABLE_BLOCKS" OFF)
add_feature_info("  Event" ENABLE_BLOCKS_EVENT "Signal/Slot event system interface blocks")
if (NOT ENABLE_BLOCKS_EVENT)
    return()
endif()

########################################################################
# Event blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET EventBlocks
    SOURCES
        PeriodicTrigger.cpp
        TestPeriodicTrigger.cpp
        MessageToSignal.cpp
        SlotToMessage.cpp
        LabelToMessage.cpp
        TestSignalsAndSlots.cpp
        Evaluator.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
