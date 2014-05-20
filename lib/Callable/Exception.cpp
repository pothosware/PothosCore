// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Callable/Exception.hpp>
#include <typeinfo>

namespace Pothos
{
POTHOS_IMPLEMENT_EXCEPTION(CallableArgumentError, InvalidArgumentException, "Callable Argument Error")
POTHOS_IMPLEMENT_EXCEPTION(CallableReturnError, NotImplementedException, "Callable Return Error")
POTHOS_IMPLEMENT_EXCEPTION(CallableNullError, NullValueException, "Callable Null Error")
} //namespace Pothos
