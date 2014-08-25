########################################################################
# Build time domain plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET WaveMonitor
    SOURCES
        WaveMonitor.cpp
        WaveMonitorWork.cpp
    DOC_SOURCES WaveMonitor.hpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
)
