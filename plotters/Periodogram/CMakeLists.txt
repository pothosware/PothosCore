########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_PLOTTERS_PERIODOGRAM "Enable Pothos Plotters.Periodogram component" ON "SPUCE_FOUND;ENABLE_PLOTTERS" OFF)
add_feature_info("  Periodogram" ENABLE_PLOTTERS_PERIODOGRAM "Periodogram frequency-domain FFT plotter")
if (NOT ENABLE_PLOTTERS_PERIODOGRAM)
    return()
endif()

########################################################################
# Build frequency domain plot module
########################################################################
include_directories(${SPUCE_INCLUDE_DIRS})

POTHOS_MODULE_UTIL(
    TARGET Periodogram
    SOURCES
        Periodogram.cpp
        PeriodogramWork.cpp
        PeriodogramChannel.cpp
        PeriodogramDisplay.cpp
    DOC_SOURCES Periodogram.cpp
    LIBRARIES
        ${Qt5_LIBRARIES}
        ${SPUCE_LIBRARIES}
    DESTINATION plotters
)
