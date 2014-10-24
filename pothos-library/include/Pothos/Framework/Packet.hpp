///
/// \file Framework/Packet.hpp
///
/// Definition for packet type found in asynchronous messages.
///
/// \copyright
/// Copyright (c) 2014-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Framework/Label.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <string>
#include <vector>

namespace Pothos {

/*!
 * The Packet type is a general-purpose structure for asynchronous messages.
 * A Packet contains a payload buffer with associated metadata and labels.
 * Ideally, the Packet structure is a suitable container for most situations.
 */
struct POTHOS_API Packet
{
    //! Default constructor for Packet
    Packet(void);

    /*!
     * The buffer payload for this message.
     * The payload is just a memory buffer and a length in bytes.
     * Payload can contains a slice of a continuous byte stream,
     * a UDP datagram, a packet in a MAC layer protocol, etc...
     * The interpretation of this payload buffer is up to the user.
     */
    BufferChunk payload;

    /*!
     * Arbitrary metadata for this message.
     * Use metadata to associate arbitrary information with the payload.
     * The are no specific requirements about what goes into metadata.
     * The metadata structure is a dictionary of key-value pairs,
     * where the keys for this dictionary are exclusively strings,
     * and the values are opaque Objects which can contain anything.
     */
    ObjectKwargs metadata;

    /*!
     * Labels associated with the payload.
     * Each label index is an element-offset relative to the start of the payload.
     * The element size can be determined from the type of the payload buffer.
     */
    std::vector<Label> labels;
};

} //namespace Pothos
