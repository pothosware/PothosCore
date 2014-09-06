///
/// \file Framework/Containers.hpp
///
/// Definitions for commonly used container types found in
/// asynchronous messages and inline labels.
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
 * The MessageGram type is a general-purpose structure for asynchronous messages.
 * A MessageGram contains a payload buffer with associated metadata and labels.
 * Ideally, the MessageGram structure is a suitable container for most situations.
 */
struct POTHOS_API MessageGram
{
    //! Default constructor for MessageGram
    MessageGram(void);

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
     * The label indexes are byte-offsets
     * relative to the start of the buffer.
     */
    std::vector<Label> labels;
};

} //namespace Pothos
