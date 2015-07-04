########################################################################
# Utility blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET UtilityBlocks
    SOURCES
        Delay.cpp
        TestDelay.cpp
        Pacer.cpp
        Converter.cpp
        TestConverter.cpp
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
        SporadicLabeler.cpp
        SporadicDropper.cpp
        FiniteRelease.cpp
        StreamSnooper.cpp
        InfiniteSource.cpp
        VectorSource.cpp
        TransformSignal.cpp
        Threshold.cpp
        Relabeler.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
