########################################################################
# Build frequency domain plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET Periodogram
    SOURCES
        Periodogram.cpp
        PeriodogramWork.cpp
        PeriodogramChannel.cpp
        PeriodogramDisplay.cpp
    DOC_SOURCES Periodogram.cpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
)
