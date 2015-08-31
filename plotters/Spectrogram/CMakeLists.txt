########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_PLOTTERS_SPECTROGRAM "Enable Pothos Plotters.Spectrogram component" ON "ENABLE_PLOTTERS" OFF)
add_feature_info("  Spectrogram" ENABLE_PLOTTERS_SPECTROGRAM "Spectrogram frequency-domain FFT plotter")
if (NOT ENABLE_PLOTTERS_SPECTROGRAM)
    return()
endif()

########################################################################
# Build spectrogram plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET Spectrogram
    SOURCES
        Spectrogram.cpp
        SpectrogramWork.cpp
        SpectrogramDisplay.cpp
    DOC_SOURCES Spectrogram.cpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION plotters
)
