########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.8.9)
project(PothosPlotters)
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
cmake_dependent_option(ENABLE_PLOTTERS "Enable Pothos Plotters component" ON "ENABLE_LIBRARY;${Qt5Enables}" OFF)
add_feature_info(Plotters ENABLE_PLOTTERS "Graphical plotter widgets for the GUI")
if (NOT ENABLE_PLOTTERS)
    return()
endif()

########################################################################
# Qwt setup
########################################################################
find_package(Qwt)

add_definitions(-DQWT_DLL)

if (QWT_FOUND)
    message(STATUS "QWT_VERSION_STRING: ${QWT_VERSION_STRING}")
    if (QWT_VERSION_STRING LESS "6.1")
        set(QWT_FOUND FALSE)
        message(STATUS "Qwt older than v6.1 - using built-in Qwt")
    endif()
else (QWT_FOUND)
    message(STATUS "Qwt not found - using built-in Qwt")
endif (QWT_FOUND)

if (QWT_FOUND)

    include_directories(${QWT_INCLUDE_DIRS})
    list(APPEND Qt5_LIBRARIES ${QWT_LIBRARIES})
    message(STATUS "QWT_INCLUDE_DIRS: ${QWT_INCLUDE_DIRS}")
    message(STATUS "QWT_LIBRARIES: ${QWT_LIBRARIES}")

else (QWT_FOUND)

    set(QWT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qwt-6.1.2/src)

    if (NOT EXISTS ${QWT_SRC_DIR})
        message(FATAL_ERROR "Qwt submodule not configured (git submodule init && git submodule update)")
    endif ()

    include_directories(${QWT_SRC_DIR})
    file(GLOB QWT_SOURCES "${QWT_SRC_DIR}/*.cpp")
    add_library(PothosQwt SHARED ${QWT_SOURCES})
    set_target_properties(PothosQwt PROPERTIES DEFINE_SYMBOL "QWT_MAKEDLL")
    set_target_properties(PothosQwt PROPERTIES VERSION 0)
    target_link_libraries(PothosQwt ${Qt5_LIBRARIES})
    install(TARGETS PothosQwt
        LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
        ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
        RUNTIME DESTINATION bin              # .dll file
    )
    list(APPEND Qt5_LIBRARIES PothosQwt)

endif (QWT_FOUND)

########################################################################
# plotter utils library
########################################################################
file(GLOB PLOTTER_UTILS_SOURCES "PlotterUtils/*.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/PlotterUtils)
add_library(PothosPlotterUtils SHARED ${PLOTTER_UTILS_SOURCES})
set_target_properties(PothosPlotterUtils PROPERTIES DEFINE_SYMBOL "POTHOS_PLOTTER_UTILS_MAKEDLL")
set_target_properties(PothosPlotterUtils PROPERTIES VERSION 0)
target_link_libraries(PothosPlotterUtils ${Qt5_LIBRARIES})
install(TARGETS PothosPlotterUtils
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)
list(APPEND Qt5_LIBRARIES PothosPlotterUtils)

########################################################################
# Build plotter widgets
########################################################################
add_subdirectory(QwtWidgets)
add_subdirectory(WaveMonitor)
add_subdirectory(Periodogram)
add_subdirectory(Spectrogram)
add_subdirectory(Constellation)
add_subdirectory(LogicAnalyzer)
