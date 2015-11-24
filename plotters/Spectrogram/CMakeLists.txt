########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_PLOTTERS_SPECTROGRAM "Enable Pothos Plotters.Spectrogram component" ON "ENABLE_PLOTTERS;Spuce_FOUND" OFF)
add_feature_info("  Spectrogram" ENABLE_PLOTTERS_SPECTROGRAM "Spectrogram frequency-domain FFT plotter")
if (NOT ENABLE_PLOTTERS_SPECTROGRAM)
    return()
endif()

########################################################################
# Build spectrogram plot module
########################################################################
include_directories(${Spuce_INCLUDE_DIRS})

POTHOS_MODULE_UTIL(
    TARGET Spectrogram
    SOURCES
        Spectrogram.cpp
        SpectrogramWork.cpp
        SpectrogramDisplay.cpp
    DOC_SOURCES Spectrogram.cpp
    LIBRARIES
        ${Qt5_LIBRARIES}
        ${Spuce_LIBRARIES}
    DESTINATION plotters
)
