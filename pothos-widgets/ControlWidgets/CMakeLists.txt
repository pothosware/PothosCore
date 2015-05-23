########################################################################
# Build control widgets module
########################################################################
set(SOURCES
    Slider.cpp
    NumericEntry.cpp
    TextDisplay.cpp
    DropDown.cpp
    RadioGroup.cpp
    PlanarSelect.cpp
    QwtSlider.cpp
    ChatBox.cpp
    PushButton.cpp
)

POTHOS_MODULE_UTIL(
    TARGET ControlWidgets
    SOURCES ${SOURCES}
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
    ENABLE_DOCS
)
