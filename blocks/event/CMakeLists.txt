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
        TestSignalsAndSlots.cpp
        Evaluator.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
