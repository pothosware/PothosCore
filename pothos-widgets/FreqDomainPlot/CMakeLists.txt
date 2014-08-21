########################################################################
# Build frequency domain plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET FreqDomainPlot
    SOURCES
        FreqDomainPlot.cpp
        FreqDomainPlotWork.cpp
    DOC_SOURCES FreqDomainPlot.hpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
)
