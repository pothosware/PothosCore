########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_PLOTTERS_CONSTELLATION "Enable Pothos Plotters.Constellation component" ON "ENABLE_PLOTTERS" OFF)
add_feature_info("  Constellation" ENABLE_PLOTTERS_CONSTELLATION "Constellation complex IQ data plotter")
if (NOT ENABLE_PLOTTERS_CONSTELLATION)
    return()
endif()

########################################################################
# Build constellation plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET Constellation
    SOURCES
        Constellation.cpp
        ConstellationWork.cpp
        ConstellationDisplay.cpp
    DOC_SOURCES Constellation.cpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION plotters
)
