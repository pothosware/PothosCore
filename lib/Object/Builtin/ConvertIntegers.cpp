// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Object/Builtin/ConvertCommonImpl.hpp"

pothos_static_block(pothosObjectRegisterConvertIntegers)
{
    //convert from intermediate
    declare_number_conversion1(sllong)
    declare_number_conversion1(ullong)
}
