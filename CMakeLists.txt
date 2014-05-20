list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(OpenCL)


message(STATUS "OPENCL_FOUND: ${OPENCL_FOUND}")
message(STATUS "OPENCL_INCLUDE_DIRS: ${OPENCL_INCLUDE_DIRS}")
message(STATUS "OPENCL_LIBRARIES: ${OPENCL_LIBRARIES}")

if(OPENCL_FOUND)
    include_directories(${OPENCL_INCLUDE_DIRS})
    list(APPEND libs ${OPENCL_LIBRARIES})
else()
    return()
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/add_numbers.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/add_numbers.cpp
@ONLY)

add_executable(opencl_example ${CMAKE_CURRENT_BINARY_DIR}/add_numbers.cpp)
target_link_libraries(opencl_example ${libs})
