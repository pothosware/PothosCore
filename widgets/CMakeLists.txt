########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosWidgets CXX)

find_package(Pothos CONFIG REQUIRED)

#args to set the CMAKE_PREFIX_PATH environment variable
if (CMAKE_PREFIX_PATH)
    set(ENV{CMAKE_PREFIX_PATH} ${CMAKE_PREFIX_PATH})
endif (CMAKE_PREFIX_PATH)

########################################################################
# Serialization library
########################################################################
if(NOT PothosSerialization_IN_TREE)
    find_package(PothosSerialization CONFIG)
endif(NOT PothosSerialization_IN_TREE)
message(STATUS "PothosSerialization_FOUND: ${PothosSerialization_FOUND} - ${PothosSerialization_VERSION}")
message(STATUS "PothosSerialization_INCLUDE_DIRS: ${PothosSerialization_INCLUDE_DIRS}")
message(STATUS "PothosSerialization_LIBRARIES: ${PothosSerialization_LIBRARIES}")

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
cmake_dependent_option(ENABLE_WIDGETS "Enable Pothos Widgets component" ON "Pothos_FOUND;${Qt5Enables};PothosSerialization_FOUND" OFF)
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

include_directories(${PothosSerialization_INCLUDE_DIRS})

POTHOS_MODULE_UTIL(
    TARGET ControlWidgets
    SOURCES ${SOURCES}
    LIBRARIES
        ${Qt5_LIBRARIES}
        ${PothosSerialization_LIBRARIES}
    DESTINATION widgets
    ENABLE_DOCS
)
