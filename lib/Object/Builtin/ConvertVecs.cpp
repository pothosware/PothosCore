// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Object/Builtin/ConvertCommonImpl.hpp"
#include <Pothos/Plugin.hpp>
#include <Pothos/Callable.hpp>
#include <complex>
#include <vector>

/***********************************************************************
 * template comprehension to handle vectors of numbers
 **********************************************************************/
template <typename InType, typename OutType>
std::vector<OutType> convertVec(const std::vector<InType> &in)
{
    std::vector<OutType> out(in.size());
    for (size_t i = 0; i < out.size(); i++)
    {
        out[i] = convertNum<InType, OutType>(in[i]);
    }
    return out;
}

/***********************************************************************
 * helper function registers a converter for specific types
 **********************************************************************/
static void registerConvertVec(const std::string &inName, const std::string &outName, const Pothos::Callable &callable)
{
    const std::string name = inName + "_to_" + outName;
    auto path = Pothos::PluginPath("/object/convert/vectors").join(name);
    Pothos::PluginRegistry::add(path, callable);
}

/***********************************************************************
 * macros to declare all conversion combinations
 **********************************************************************/
#define declare_vector_conversion2(inType, outType) \
    registerConvertVec(#inType, #outType, Pothos::Callable(&convertVec<inType, outType>));

#define declare_vector_conversion1(inType) \
    declare_vector_conversion2(inType, char) \
    declare_vector_conversion2(inType, uchar) \
    declare_vector_conversion2(inType, schar) \
    declare_vector_conversion2(inType, ushort) \
    declare_vector_conversion2(inType, sshort) \
    declare_vector_conversion2(inType, uint) \
    declare_vector_conversion2(inType, sint) \
    declare_vector_conversion2(inType, ulong) \
    declare_vector_conversion2(inType, slong) \
    declare_vector_conversion2(inType, ullong) \
    declare_vector_conversion2(inType, sllong) \
    declare_vector_conversion2(inType, float) \
    declare_vector_conversion2(inType, double) \
    declare_vector_conversion2(inType, cfloat) \
    declare_vector_conversion2(inType, cdouble)

#define declare_vector_conversion() \
    declare_vector_conversion1(char) \
    declare_vector_conversion1(uchar) \
    declare_vector_conversion1(schar) \
    declare_vector_conversion1(ushort) \
    declare_vector_conversion1(sshort) \
    declare_vector_conversion1(uint) \
    declare_vector_conversion1(sint) \
    declare_vector_conversion1(ulong) \
    declare_vector_conversion1(slong) \
    declare_vector_conversion1(ullong) \
    declare_vector_conversion1(sllong) \
    declare_vector_conversion1(float) \
    declare_vector_conversion1(double) \
    declare_vector_conversion1(cfloat) \
    declare_vector_conversion1(cdouble)

/***********************************************************************
 * all registration code is called from here
 **********************************************************************/
pothos_static_block(pothosObjectRegisterConvertVecs)
{
    declare_vector_conversion()
}

