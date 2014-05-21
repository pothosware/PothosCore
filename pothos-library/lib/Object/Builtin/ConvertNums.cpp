// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Object/Builtin/ConvertCommonImpl.hpp"
#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>
#include <complex>

/***********************************************************************
 * helper function registers a converter for specific types
 **********************************************************************/
static void registerConvertNum(const std::string &inName, const std::string &outName, const Pothos::Callable &callable)
{
    const std::string name = inName + "_to_" + outName;
    auto path = Pothos::PluginPath("/object/convert/numbers").join(name);
    Pothos::PluginRegistry::add(path, callable);
}

/***********************************************************************
 * macros to declare all conversion combinations
 **********************************************************************/
#define declare_number_conversion2(inType, outType) \
    registerConvertNum(#inType, #outType, Pothos::Callable(&convertNum<inType, outType>));

#define declare_number_conversion1(inType) \
    declare_number_conversion2(inType, char) \
    declare_number_conversion2(inType, uchar) \
    declare_number_conversion2(inType, schar) \
    declare_number_conversion2(inType, ushort) \
    declare_number_conversion2(inType, sshort) \
    declare_number_conversion2(inType, uint) \
    declare_number_conversion2(inType, sint) \
    declare_number_conversion2(inType, ulong) \
    declare_number_conversion2(inType, slong) \
    declare_number_conversion2(inType, ullong) \
    declare_number_conversion2(inType, sllong) \
    declare_number_conversion2(inType, float) \
    declare_number_conversion2(inType, double) \
    declare_number_conversion2(inType, cfloat) \
    declare_number_conversion2(inType, cdouble)

#define declare_number_conversion() \
    declare_number_conversion1(char) \
    declare_number_conversion1(uchar) \
    declare_number_conversion1(schar) \
    declare_number_conversion1(ushort) \
    declare_number_conversion1(sshort) \
    declare_number_conversion1(uint) \
    declare_number_conversion1(sint) \
    declare_number_conversion1(ulong) \
    declare_number_conversion1(slong) \
    declare_number_conversion1(ullong) \
    declare_number_conversion1(sllong) \
    declare_number_conversion1(float) \
    declare_number_conversion1(double) \
    declare_number_conversion1(cfloat) \
    declare_number_conversion1(cdouble)

/***********************************************************************
 * all registration code is called from here
 **********************************************************************/
pothos_static_block(pothosObjectRegisterConvertNums)
{
    declare_number_conversion()
}
