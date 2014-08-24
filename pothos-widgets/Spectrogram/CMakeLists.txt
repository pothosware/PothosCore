########################################################################
# Build spectrogram plot module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET Spectrogram
    SOURCES
        Spectrogram.cpp
        SpectrogramWork.cpp
    DOC_SOURCES Spectrogram.hpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
)
