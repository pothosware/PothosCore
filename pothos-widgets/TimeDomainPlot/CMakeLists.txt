########################################################################
# Build time domain plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET TimeDomainPlot
    SOURCES
        TimeDomainPlot.cpp
        TimeDomainPlotWork.cpp
    DOC_SOURCES TimeDomainPlot.hpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
)
