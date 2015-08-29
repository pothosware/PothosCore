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
