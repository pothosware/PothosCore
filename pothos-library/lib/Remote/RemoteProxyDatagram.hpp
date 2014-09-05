// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Poco/Base64Encoder.h>
#include <Poco/Base64Decoder.h>

inline void sendDatagram(std::ostream &os, const Pothos::ObjectKwargs &reqArgs)
{
    Pothos::Object request(reqArgs);
    Poco::Base64Encoder encoder(os);
    request.serialize(encoder);
    encoder.close();
}

inline Pothos::ObjectKwargs recvDatagram(std::istream &is)
{
    Poco::Base64Decoder decoder(is);
    Pothos::Object reply;
    reply.deserialize(decoder);
    return reply.extract<Pothos::ObjectKwargs>();
}
