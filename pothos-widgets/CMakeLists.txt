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

foreach(comp Widgets Concurrent OpenGL Svg PrintSupport)
    find_package(Qt5${comp})

    if(NOT Qt5${comp}_FOUND)
        MESSAGE(WARNING "Cant find Qt5${comp}")
        return()
    endif()

    include_directories(${Qt5${comp}_INCLUDE_DIRS})
    add_definitions(${Qt5${comp}_DEFINITIONS})
    list(APPEND Qt5_LIBRARIES ${Qt5${comp}_LIBRARIES})
endforeach(comp)

########################################################################
# Qwt setup
########################################################################
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/qwt/src)
add_definitions(-DQWT_DLL)
file(GLOB QWT_SOURCES "qwt/src/*.cpp")
add_library(PothosQwt SHARED ${QWT_SOURCES})
set_target_properties(PothosQwt PROPERTIES DEFINE_SYMBOL "QWT_MAKEDLL")
target_link_libraries(PothosQwt ${Qt5_LIBRARIES})
install(TARGETS PothosQwt
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)
list(APPEND Qt5_LIBRARIES PothosQwt)

########################################################################
# Build widgets module
########################################################################
set(SOURCES
    QStringSupport.cpp
    Slider.cpp
    NumericEntry.cpp
    TextDisplay.cpp
    SliderEntry.cpp
    QwtSlider.cpp
    TimeDomainPlot/TimeDomainPlot.hpp #scanned for block desc
    TimeDomainPlot/TimeDomainPlot.cpp
)

POTHOS_MODULE_UTIL(
    TARGET GeneralWidgets
    SOURCES ${SOURCES}
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
    ENABLE_DOCS
)
