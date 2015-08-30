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
