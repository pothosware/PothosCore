///
/// \file Plugin/Static.hpp
///
/// Static initialization implementation for load-time registration.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/System/Version.hpp>

/*!
 * Define the tokens used to declare a static block's function.
 * The address of the function is used to make the static fixture
 * class unique across the complete library + module address space.
 *
 * Clang can only use static, inline functions of the same name will overlap.
 * MSVC can only use inline because static cannot be used as a template argument.
 * And GCC can handle all possibilities: static, inline, and static inline.
 *
 * The specific reason for this implementation and its caveats
 * is because clang will cause symbol overlap for class and functions
 * with the same name across modules, even if they are not exported.
 */
#if defined(__clang__)
#define POTHOS_STATIC_FIXTURE_DECL static
#elif defined(_MSC_VER)
#define POTHOS_STATIC_FIXTURE_DECL inline
#elif defined(__GNUC__)
#define POTHOS_STATIC_FIXTURE_DECL static inline
#else
#define POTHOS_STATIC_FIXTURE_DECL
#endif

/*!
 * Define POTHOS_PROJECT used as the fixture's outer namespace.
 * The POTHOS_MODULE_UTIL() will set this to the project name.
 */
#ifndef POTHOS_PROJECT
#define POTHOS_PROJECT PothosProject
#endif

/*!
 * Define POTHOS_TARGET used as the fixture's inner namespace.
 * The POTHOS_MODULE_UTIL() will set this to the target name.
 */
#ifndef POTHOS_TARGET
#define POTHOS_TARGET PothosTarget
#endif

/*!
 * pothos_static_block - macro that declares code to be run at init time
 * Example usage:
 * pothos_static_block(someValidFunctionName)
 * {
 *      //your code here to be run at init time
 * }
 */
#define pothos_static_block(name) \
    namespace POTHOS_PROJECT { namespace POTHOS_TARGET { \
    POTHOS_STATIC_FIXTURE_DECL void name ## StaticFixtureInit__(void); \
    template <Pothos::Detail::InitFcn init> \
    struct name ## StaticFixture__ \
    { \
        name ## StaticFixture__(void) \
        { \
            Pothos::Detail::safeInit(POTHOS_ABI_VERSION, #name, init); \
        } \
    }; \
    static name ## StaticFixture__<&name ## StaticFixtureInit__> name ## StaticFixtureInstance__; }} \
    POTHOS_STATIC_FIXTURE_DECL void POTHOS_PROJECT::POTHOS_TARGET::name ## StaticFixtureInit__(void)

namespace Pothos {
namespace Detail {

typedef void (*InitFcn)(void);

POTHOS_API void safeInit(const std::string &clientAbi, const std::string &name, InitFcn init);

} //namespace Detail
} //namespace Pothos
