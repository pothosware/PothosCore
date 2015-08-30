########################################################################
# Waveform blocks module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET WaveformBlocks
    SOURCES
        WaveformSource.cpp
        NoiseSource.cpp
    DESTINATION blocks
    ENABLE_DOCS
)
