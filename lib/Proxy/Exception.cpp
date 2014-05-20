// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Proxy/Exception.hpp>
#include <typeinfo>

namespace Pothos
{
POTHOS_IMPLEMENT_EXCEPTION(ProxyEnvironmentFactoryError, RuntimeException, "Proxy Environment Factory Error")
POTHOS_IMPLEMENT_EXCEPTION(ProxyEnvironmentFindError, RuntimeException, "Proxy Environment Find Error")
POTHOS_IMPLEMENT_EXCEPTION(ProxyEnvironmentConvertError, RuntimeException, "Proxy Environment Convert Error")
POTHOS_IMPLEMENT_EXCEPTION(ProxyHandleCallError, RuntimeException, "Proxy Handle Call Error")
POTHOS_IMPLEMENT_EXCEPTION(ProxyExceptionMessage, RuntimeException, "Proxy Exception Message")
POTHOS_IMPLEMENT_EXCEPTION(ProxyCompareError, RuntimeException, "Proxy Compare Error")
} //namespace Pothos
