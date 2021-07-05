// Copyright (c) 2021 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Poco/Platform.h>

// Only supported in MSVC 2017+ (no C++11 flag)
#if defined(POCO_COMPILER_MSVC) && _MSC_VER >= 1910
    #define CPP11_COMPILER_FLAG ""
    #define CPP14_COMPILER_FLAG "/std:c++14"
    #define CPP17_COMPILER_FLAG "/std:c++17"
    #define CPP20_COMPILER_FLAG "/std:c++latest"
#elif defined(POCO_COMPILER_GCC) || defined(POCO_COMPILER_MINGW) || defined(POCO_COMPILER_CLANG)
    #define CPP11_COMPILER_FLAG "-std=c++11"
    #define CPP14_COMPILER_FLAG "-std=c++14"
    #define CPP17_COMPILER_FLAG "-std=c++17"
    #define CPP20_COMPILER_FLAG "-std=c++20"
#else
    #define CPP11_COMPILER_FLAG ""
    #define CPP14_COMPILER_FLAG ""
    #define CPP17_COMPILER_FLAG ""
    #define CPP20_COMPILER_FLAG ""
#endif

#ifdef POCO_COMPILER_MSVC
#define CPP_STD _MSVC_LANG
#else
#define CPP_STD __cplusplus
#endif

#if CPP_STD >= 202002L
#define CPP_STD_FLAG CPP20_COMPILER_FLAG
#elif CPP_STD >= 201703L
#define CPP_STD_FLAG CPP17_COMPILER_FLAG
#elif CPP_STD >= 201402L
#define CPP_STD_FLAG CPP14_COMPILER_FLAG
#else
#define CPP_STD_FLAG CPP11_COMPILER_FLAG
#endif
