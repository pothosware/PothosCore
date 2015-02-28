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
find_package(Qwt)

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

endif (QWT_FOUND)

########################################################################
# plotter utils library
########################################################################
file(GLOB WIDGET_UTILS_SOURCES "WidgetUtils/*.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/WidgetUtils)
add_library(PothosWidgetUtils SHARED ${WIDGET_UTILS_SOURCES})
set_target_properties(PothosWidgetUtils PROPERTIES DEFINE_SYMBOL "POTHOS_WIDGET_UTILS_MAKEDLL")
target_link_libraries(PothosWidgetUtils ${Qt5_LIBRARIES})
install(TARGETS PothosWidgetUtils
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)
list(APPEND Qt5_LIBRARIES PothosWidgetUtils)

########################################################################
# Build widgets module
########################################################################
POTHOS_MODULE_UTIL(
    TARGET GeneralWidgets
    SOURCES QStringSupport.cpp
    LIBRARIES ${Qt5_LIBRARIES}
    DESTINATION widgets
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(ControlWidgets)
add_subdirectory(WaveMonitor)
add_subdirectory(Periodogram)
add_subdirectory(Spectrogram)
add_subdirectory(Constellation)
