// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Object/Builtin/ConvertCommonImpl.hpp"

pothos_static_block(pothosObjectRegisterConvertIntermediate)
{
    //convert to intermediate
    declare_number_conversion2(char, sllong)
    declare_number_conversion2(uchar, ullong)
    declare_number_conversion2(schar, sllong)
    declare_number_conversion2(ushort, ullong)
    declare_number_conversion2(sshort, sllong)
    declare_number_conversion2(uint, ullong)
    declare_number_conversion2(sint, sllong)
    declare_number_conversion2(ulong, ullong)
    declare_number_conversion2(slong, sllong)
    declare_number_conversion2(float, double)
    declare_number_conversion2(cfloat, cdouble)

    //integer based complex conversions
    declare_number_conversion2(cschar, csllong)
    declare_number_conversion2(csshort, csllong)
    declare_number_conversion2(csint, csllong)
    declare_number_conversion2(cslong, csllong)
    declare_number_conversion2(csllong, cdouble)
}
