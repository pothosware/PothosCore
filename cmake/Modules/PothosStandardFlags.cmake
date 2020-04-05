if(DEFINED INCLUDED_POTHOS_STANDARD_FLAGS_CMAKE)
    return()
endif()
set(INCLUDED_POTHOS_STANDARD_FLAGS_CMAKE TRUE)

########################################################################
# Compiler flags that are generally always a good idea to set
# or sensible to use because of the project's constraints.
########################################################################

if(NOT CMAKE_CXX_STANDARD)
    # C++14 is a required language feature for this project
    set(CMAKE_CXX_STANDARD 14)
else()
    if(CMAKE_CXX_STANDARD LESS 14)
        message(WARNING "Specified standard C++${CMAKE_CXX_STANDARD} is less than the official minimum standard C++14. Support is not guaranteed.")
    endif()
    message(STATUS "Overriding C++ standard to C++${CMAKE_CXX_STANDARD}.")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    #force a compile-time error when symbols are missing
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined")

    #these warnings are caused by static warnings used throughout the code
    add_compile_options(-Wno-unused-local-typedefs)

    #common warnings to help encourage good coding practices
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Wnon-virtual-dtor)

    #symbols are only exported from libraries/modules explicitly
    add_compile_options(-fvisibility=hidden)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

if(APPLE)
    #fixes issue with duplicate module registry when using application bundle
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flat_namespace")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -flat_namespace")
endif()

if(MSVC)
    if(CMAKE_CXX_STANDARD EQUAL 14)
        # C++14 is a required language feature for this project
        if (${MSVC_VERSION} LESS 1910)
            message(FATAL_ERROR "the build requires VS 2017 15.0 or newer for C++14 support")
        endif()
    elseif(CMAKE_CXX_STANDARD EQUAL 17)
        if (${MSVC_VERSION} LESS 1914)
            message(FATAL_ERROR "the build requires VS 2017 15.7 or newer for C++17 support")
        endif()
    endif()

    #we always want to use multiple cores for compilation
    add_compile_options(/MP)

    #projects should be cross-platform and standard stl functions should work
    add_definitions(-DNOMINMAX) #enables std::min and std::max

    #suppress the following warnings which are commonly caused by project headers
    add_compile_options(/wd4251) #disable 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
    add_compile_options(/wd4275) #disable non – DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
    add_compile_options(/wd4503) #'identifier' : decorated name length exceeded, name was truncated

else(MSVC)
    #define _DEBUG for debug mode flags (used by some macros)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

endif(MSVC)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
    add_compile_options(-stdlib=libc++)
endif()
