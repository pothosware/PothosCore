########################################################################
## Feature registration
########################################################################
cmake_dependent_option(ENABLE_LIBRARY_INCLUDE "Enable Pothos Library.Include component" ON "ENABLE_LIBRARY" OFF)
add_feature_info("  Include" ENABLE_LIBRARY_INCLUDE "Public C++ headers for Pothos library")
if (NOT ENABLE_LIBRARY_INCLUDE)
    return()
endif()

########################################################################
# install include files
########################################################################
install(
    DIRECTORY Pothos
    DESTINATION include
    COMPONENT pothos_devel
)
