########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosWidgets)
enable_language(CXX)

find_package(Pothos CONFIG REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

########################################################################
# QT5 devel setup
########################################################################
#http://www.kdab.com/using-cmake-with-qt-5/
# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

unset(Qt5Enables)
foreach(comp Widgets Concurrent OpenGL Svg PrintSupport)
    find_package(Qt5${comp})
    set(Qt5Enables "${Qt5Enables};Qt5${comp}_FOUND")

    if(Qt5${comp}_FOUND)
        include_directories(${Qt5${comp}_INCLUDE_DIRS})
        add_definitions(${Qt5${comp}_DEFINITIONS})
        list(APPEND Qt5_LIBRARIES ${Qt5${comp}_LIBRARIES})
    else()
        MESSAGE(WARNING "Cant find Qt5${comp}")
    endif()
endforeach(comp)

########################################################################
## Feature registration
########################################################################
include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_WIDGETS "Enable Pothos Widgets component" ON "ENABLE_LIBRARY;${Qt5Enables}" OFF)
add_feature_info(Widgets ENABLE_WIDGETS "Graphical control widgets for the GUI")
if (NOT ENABLE_WIDGETS)
    return()
endif()

########################################################################
# Build control widgets module
########################################################################
set(SOURCES
    QStringSupport.cpp
    Slider.cpp
    NumericEntry.cpp
    TextDisplay.cpp
    DropDown.cpp
    RadioGroup.cpp
    PlanarSelect.cpp
    ChatBox.cpp
    PushButton.cpp
    MyDoubleSlider.cpp
)

POTHOS_MODULE_UTIL(
    TARGET ControlWidgets
    SOURCES ${SOURCES}
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
    ENABLE_DOCS
)
