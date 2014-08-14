########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosWidgets)
enable_language(CXX)

if (NOT POTHOS_MODULE_PATH)
    set(POTHOS_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/Pothos/cmake/Modules")
endif()

include(${POTHOS_MODULE_PATH}/PothosUtil.cmake)

########################################################################
# QT5 devel setup
########################################################################
#http://www.kdab.com/using-cmake-with-qt-5/
# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Widgets finds its own dependencies.
find_package(Qt5Widgets)

if(NOT Qt5Widgets_FOUND)
    return()
endif()
include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})
list(APPEND POTHOS_LIBRARIES ${Qt5Widgets_LIBRARIES})

########################################################################
# Build widgets module
########################################################################
set(SOURCES
    Slider.cpp
    TimeDomainPlot/TimeDomainPlot.hpp #scanned for block desc
    TimeDomainPlot/TimeDomainPlot.cpp
)

POTHOS_MODULE_UTIL(
    TARGET GeneralWidgets
    SOURCES ${SOURCES}
    LIBRARIES ${POTHOS_LIBRARIES}
    DESTINATION widgets
    ENABLE_DOCS
)
