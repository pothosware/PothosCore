########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_PLOTTERS_QWTWIDGETS "Enable Pothos Plotters.QwtWidgets component" ON "ENABLE_PLOTTERS" OFF)
add_feature_info("  Qwt Widgets" ENABLE_PLOTTERS_QWTWIDGETS "Miscellaneous widgets based around Qwt")
if (NOT ENABLE_PLOTTERS_QWTWIDGETS)
    return()
endif()

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
