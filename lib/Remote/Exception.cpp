// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Remote/Exception.hpp>
#include <typeinfo>

namespace Pothos
{
POTHOS_IMPLEMENT_EXCEPTION(RemoteNodeError, RuntimeException, "Remote Node Error")
POTHOS_IMPLEMENT_EXCEPTION(RemoteClientError, RuntimeException, "Remote Client Error")
POTHOS_IMPLEMENT_EXCEPTION(RemoteServerError, RuntimeException, "Remote Server Error")
} //namespace Pothos
