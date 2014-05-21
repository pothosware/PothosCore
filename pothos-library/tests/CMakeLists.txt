########################################################################
# Setup Boost defines that get used before find_package(Boost)
########################################################################
if(UNIX AND NOT BOOST_ROOT AND EXISTS "/usr/lib64" AND NOT BOOST_LIBRARYDIR)
    list(APPEND BOOST_LIBRARYDIR "/usr/lib64") #fedora 64-bit fix
endif(UNIX AND NOT BOOST_ROOT AND EXISTS "/usr/lib64" AND NOT BOOST_LIBRARYDIR)

set(Boost_ADDITIONAL_VERSIONS
    "1.35.0" "1.35" "1.36.0" "1.36" "1.37.0" "1.37" "1.38.0" "1.38" "1.39.0" "1.39"
    "1.40.0" "1.40" "1.41.0" "1.41" "1.42.0" "1.42" "1.43.0" "1.43" "1.44.0" "1.44"
    "1.45.0" "1.45" "1.46.0" "1.46" "1.47.0" "1.47" "1.48.0" "1.48" "1.49.0" "1.49"
    "1.50.0" "1.50" "1.51.0" "1.51" "1.52.0" "1.52" "1.53.0" "1.53" "1.54.0" "1.54"
    "1.55.0" "1.55" "1.56.0" "1.56" "1.57.0" "1.57" "1.58.0" "1.58" "1.59.0" "1.59"
    "1.60.0" "1.60" "1.61.0" "1.61" "1.62.0" "1.62" "1.63.0" "1.63" "1.64.0" "1.64"
    "1.65.0" "1.65" "1.66.0" "1.66" "1.67.0" "1.67" "1.68.0" "1.68" "1.69.0" "1.69"
)

if(MSVC)
    add_definitions(-DBOOST_ALL_DYN_LINK) #setup boost auto-linking in msvc
endif(MSVC)

########################################################################
# unit test suite
########################################################################
find_package(Boost COMPONENTS unit_test_framework)

if(NOT Boost_FOUND)
    return()
endif()

set(test_sources
    test_object.cpp
    test_callable.cpp
    test_plugin.cpp
)

include_directories(${POTHOS_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

#turn each test cpp file into an executable with an int main() function
add_definitions(-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN)

#for each source: build an executable, register it as a test
foreach(test_source ${test_sources})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} ${Boost_LIBRARIES} ${POTHOS_LIBRARIES})
    add_test(${test_name} ${test_name})
endforeach(test_source)
