//
// Framework/OutputPort.hpp
//
// This file provides an interface for a worker's output port.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/Label.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <string>

namespace Pothos {

class OutputPortImpl;
class WorkerActor;

/*!
 * OutputPort provides methods to interact with a worker's output ports.
 */
class POTHOS_API OutputPort
{
public:

    //! Destructor
    ~OutputPort(void);

    /*!
     * Get the index number of this port.
     * An index of -1 means the port cannot be represented by an integer.
     * \return the index or -1
     */
    int index(void) const;

    //! Get the string name identifier for this port.
    const std::string &name(void) const;

    //! Get the data type information for this port.
    const DType &dtype(void) const;

    /*!
     * Get access to the stream buffer.
     * For non-stream ports, this returns an empty buffer chunk.
     */
    const BufferChunk &buffer(void) const;

    /*!
     * Get the number of elements available in the stream buffer.
     * The number of elements is the available bytes/dtype size.
     */
    size_t elements(void) const;

    /*!
     * Get the total number of elements produced from this port.
     * The value returned by this method will not change
     * until after execution of work() and propagateLabels().
     */
    unsigned long long totalElements(void) const;

    /*!
     * Get the total number of messages posted to this port.
     * The value returned by this method will be incremented
     * immediately upon calling postMessage().
     */
    unsigned long long totalMessages(void) const;

    /*!
     * Produce elements from this port.
     * The number of elements specified must be less than
     * or equal to the number of elements available.
     * \param numElements the number of elements to produce
     */
    void produce(const size_t numElements);

    /*!
     * Remove all or part of an output buffer from the port;
     * without producing elements for downstream consumers.
     * This call allows the user to use the output buffer
     * for non-streaming purposes, such as an async message.
     * The number of bytes should be less than or equal to
     * the bytes available and a multiple of the element size.
     * \param numBytes the number of bytes to remove
     */
    void popBuffer(const size_t numBytes);

    /*!
     * Post an output label to the subscribers on this port.
     * \param label the label to post
     */
    void postLabel(const Label &label);

    /*!
     * Post an output message to the subscribers on this port.
     * \param message the message to post
     */
    void postMessage(const Object &message);

    /*!
     * Post an output buffer to the subscribers on this port.
     * This call allows external user-provided buffers to be used
     * in place of the normal stream buffer provided by buffer().
     * The number of elements to produce is determined from
     * the buffer's length field divided by the dtype size.
     * Do not call produce() when using postBuffer().
     * \param buffer the buffer to post
     */
    void postBuffer(const BufferChunk &buffer);

private:
    OutputPortImpl *_impl;
    int _index;
    std::string _name;
    DType _dtype;
    BufferChunk _buffer;
    size_t _elements;
    unsigned long long _totalElements;
    unsigned long long _totalMessages;
    size_t _pendingElements;
    OutputPort(OutputPortImpl *);
    OutputPort(const OutputPort &){} // non construction-copyable
    OutputPort &operator=(const OutputPort &){return *this;} // non copyable
    friend class WorkerActor;
};

} //namespace Pothos

inline int Pothos::OutputPort::index(void) const
{
    return _index;
}

inline const std::string &Pothos::OutputPort::name(void) const
{
    return _name;
}

inline const Pothos::DType &Pothos::OutputPort::dtype(void) const
{
    return _dtype;
}

inline const Pothos::BufferChunk &Pothos::OutputPort::buffer(void) const
{
    return _buffer;
}

inline size_t Pothos::OutputPort::elements(void) const
{
    return _elements;
}

inline unsigned long long Pothos::OutputPort::totalElements(void) const
{
    return _totalElements;
}

inline unsigned long long Pothos::OutputPort::totalMessages(void) const
{
    return _totalMessages;
}

inline void Pothos::OutputPort::produce(const size_t numElements)
{
    _pendingElements += numElements;
}
