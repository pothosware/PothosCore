########################################################################
# Build frequency domain plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET Periodogram
    SOURCES
        Periodogram.cpp
        PeriodogramWork.cpp
    DOC_SOURCES Periodogram.hpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
)
