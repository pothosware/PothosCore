########################################################################
# Build Qwt widgets module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET QwtWidgets
    SOURCES
        QwtSlider.cpp
    ENABLE_DOCS
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION plotters
)
