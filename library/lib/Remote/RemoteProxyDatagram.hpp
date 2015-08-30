// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Object/Containers.hpp>
#include <iosfwd>

/*!
 * Serialize a request object to an output stream
 */
void sendDatagram(std::ostream &os, const Pothos::ObjectKwargs &reqArgs);

/*!
 * Deserialize a reply object from an input stream
 */
Pothos::ObjectKwargs recvDatagram(std::istream &is);
