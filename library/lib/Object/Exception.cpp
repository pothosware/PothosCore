// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Exception.hpp>
#include <typeinfo>

namespace Pothos
{
POTHOS_IMPLEMENT_EXCEPTION(ObjectConvertError, RuntimeException, "Object Convert Error")
POTHOS_IMPLEMENT_EXCEPTION(ObjectSerializeError, RuntimeException, "Object Serialize Error")
POTHOS_IMPLEMENT_EXCEPTION(ObjectCompareError, RuntimeException, "Object Compare Error")
} //namespace Pothos
