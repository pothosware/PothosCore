########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_PLOTTERS_WAVEMONITOR "Enable Pothos Plotters.WaveMonitor component" ON "ENABLE_PLOTTERS" OFF)
add_feature_info("  Wave Monitor" ENABLE_PLOTTERS_WAVEMONITOR "Wavemonitor time-domain waveform plotter")
if (NOT ENABLE_PLOTTERS_WAVEMONITOR)
    return()
endif()

########################################################################
# Build time domain plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET WaveMonitor
    SOURCES
        WaveMonitor.cpp
        WaveMonitorWork.cpp
        WaveMonitorDisplay.cpp
    DOC_SOURCES WaveMonitor.cpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION plotters
)
