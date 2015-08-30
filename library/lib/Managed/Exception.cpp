// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Managed/Exception.hpp>
#include <typeinfo>

namespace Pothos
{
POTHOS_IMPLEMENT_EXCEPTION(ManagedClassLookupError, RuntimeException, "Managed Class Lookup Error")
POTHOS_IMPLEMENT_EXCEPTION(ManagedClassTypeError, RuntimeException, "Managed Class Type Error")
POTHOS_IMPLEMENT_EXCEPTION(ManagedClassNameError, RuntimeException, "Managed Class Name Error")
} //namespace Pothos
