########################################################################
# Utility blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET UtilityBlocks
    SOURCES
        Delay.cpp
        Pacer.cpp
        PeriodicTrigger.cpp
        TestPeriodicTrigger.cpp
        Copier.cpp
        Gateway.cpp
        TestGateway.cpp
        MessageToSignal.cpp
        SlotToMessage.cpp
        BlackHole.cpp
        TestSignalsAndSlots.cpp
        DynamicRouter.cpp
        StreamProbe.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
