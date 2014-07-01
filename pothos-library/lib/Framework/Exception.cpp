// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Exception.hpp>
#include <typeinfo>

namespace Pothos {
POTHOS_IMPLEMENT_EXCEPTION(SharedBufferError, RuntimeException, "Framework Shared Error")
POTHOS_IMPLEMENT_EXCEPTION(BufferManagerFactoryError, RuntimeException, "Framework Buffer Manager Factory Error")
POTHOS_IMPLEMENT_EXCEPTION(BufferPushError, RuntimeException, "Framework Buffer Push Error")
POTHOS_IMPLEMENT_EXCEPTION(PortAccessError, RangeException, "Framework Worker Port Access Error")
POTHOS_IMPLEMENT_EXCEPTION(DTypeUnknownError, RuntimeException, "Framework DType Unknown Identifier Error")
POTHOS_IMPLEMENT_EXCEPTION(TopologyConnectError, RuntimeException, "Framework Topology Connect Error")
POTHOS_IMPLEMENT_EXCEPTION(BlockCallNotFound, RuntimeException, "Framework Block Call Not Found")
POTHOS_IMPLEMENT_EXCEPTION(PortDomainError, RuntimeException, "Framework Port Domain Incompatible")
} //namespace Pothos
