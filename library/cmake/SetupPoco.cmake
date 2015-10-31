if(DEFINED INCLUDED_SETUP_POCO_CMAKE)
    return()
endif()
set(INCLUDED_SETUP_POCO_CMAKE TRUE)

########################################################################
# Find Poco with PocoConfig.cmake (installed copy)
########################################################################
set(Poco_INCLUDE_DIRS "")
find_package(Poco CONFIG COMPONENTS Foundation Util JSON XML Net)
if (Poco_FOUND)
    message(STATUS "Poco_DIR: ${Poco_DIR}")
    message(STATUS "Poco_LIBRARIES: ${Poco_LIBRARIES}")
    return()
endif()

########################################################################
# Find Poco with FindPoco.cmake (local copy)
########################################################################
message(STATUS "PocoConfig not found - trying with FindPoco")
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_LIST_DIR})
find_package(Poco)
if (POCO_FOUND AND POCO_Util_FOUND AND POCO_Net_FOUND AND POCO_XML_FOUND AND POCO_JSON_FOUND)
    set(Poco_FOUND 1)
    set(Poco_INCLUDE_DIRS ${POCO_INCLUDE_DIRS})
    set(Poco_LIBRARIES
        ${POCO_LIBRARIES}
        ${POCO_Util_LIBRARIES}
        ${POCO_Net_LIBRARIES}
        ${POCO_XML_LIBRARIES}
        ${POCO_JSON_LIBRARIES}
    )
    message(STATUS "Poco_INCLUDE_DIRS: ${Poco_INCLUDE_DIRS}")
    message(STATUS "Poco_LIBRARIES: ${Poco_LIBRARIES}")

    ########################################################################
    # Link with the thread library
    ########################################################################
    if (UNIX)
        set(CMAKE_THREAD_PREFER_PTHREAD ON)
        find_package(Threads)
        list(APPEND Poco_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
    endif()
endif()
