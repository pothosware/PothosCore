// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Object/Builtin/ConvertCommonImpl.hpp"

pothos_static_block(pothosObjectRegisterConvertFloats)
{
    //convert from intermediate
    declare_number_conversion1(double)
    declare_number_conversion1(cdouble)
}
