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
