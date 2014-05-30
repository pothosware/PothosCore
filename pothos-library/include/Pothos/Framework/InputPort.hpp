//
// Framework/InputPort.hpp
//
// This file provides an interface for a worker's input port.
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

class InputPortImpl;
class WorkerActor;

/*!
 * InputPort provides methods to interact with a worker's input ports.
 */
class POTHOS_API InputPort
{
public:

    //! Destructor
    ~InputPort(void);

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
     * Get the total number of elements consumed on this port.
     * The value returned by this method will not change
     * until after execution of work() and propagateLabels().
     */
    unsigned long long totalElements(void) const;

    /*!
     * Get the total number of messages popped from this port.
     * The value returned by this method will be incremented
     * immediately upon calling popMessage().
     */
    unsigned long long totalMessages(void) const;

    //! Does the specified input port have an asynchronous message available?
    bool hasMessage(void) const;

    /*!
     * Get an iterator to all input labels for the specified port.
     * Labels are sorted in order of oldest to newest by label index.
     * \return an iterable object with sorted labels
     */
    const LabelIteratorRange &labels(void) const;

    /*!
     * Remove a label from the internal storage structure.
     * This invalidates the iterator retrieved from labels().
     * Since labels are automatically removed by consume(),
     * this call allows a users to simplify state tracking
     * in their block implementations by removing labels,
     * that might be iterated through on subsequent runs.
     */
    void removeLabel(const Label &label);

    /*!
     * Consume elements on this port.
     * The number of elements specified must be less than
     * or equal to the number of elements available.
     * \param numElements the number of elements to consume
     */
    void consume(const size_t numElements);

    /*!
     * Remove and return an asynchronous message from the port.
     * If there is no message available, a null Object() is returned.
     * \return an asynchronous message object
     */
    Object popMessage(void);

    /*!
     * Set a reserve requirement on this input port.
     * A reserve requirement means that work() will not be called
     * unless this input port has at least numElements available;
     * An exception to this rule is when a message is available.
     * By default, each input port has a reserve of zero elements.
     * \param numElements the number of elements to require
     */
    void setReserve(const size_t numElements);

    /*!
     * Is this port used for signal handling in a signals + slots paradigm?
     */
    bool isSlot(void) const;

private:
    InputPortImpl *_impl;
    int _index;
    std::string _name;
    DType _dtype;
    BufferChunk _buffer;
    size_t _elements;
    unsigned long long _totalElements;
    unsigned long long _totalMessages;
    LabelIteratorRange _labelIter;
    size_t _pendingElements;
    size_t _reserveElements;
    InputPort(InputPortImpl *);
    InputPort(const InputPort &){} // non construction-copyable
    InputPort &operator=(const InputPort &){return *this;} // non copyable
    friend class WorkerActor;
};

} //namespace Pothos

inline int Pothos::InputPort::index(void) const
{
    return _index;
}

inline const std::string &Pothos::InputPort::name(void) const
{
    return _name;
}

inline const Pothos::DType &Pothos::InputPort::dtype(void) const
{
    return _dtype;
}

inline const Pothos::BufferChunk &Pothos::InputPort::buffer(void) const
{
    return _buffer;
}

inline size_t Pothos::InputPort::elements(void) const
{
    return _elements;
}

inline unsigned long long Pothos::InputPort::totalElements(void) const
{
    return _totalElements;
}

inline unsigned long long Pothos::InputPort::totalMessages(void) const
{
    return _totalMessages;
}

inline const Pothos::LabelIteratorRange &Pothos::InputPort::labels(void) const
{
    return _labelIter;
}

inline void Pothos::InputPort::consume(const size_t numElements)
{
    _pendingElements += numElements;
}

inline void Pothos::InputPort::setReserve(const size_t numElements)
{
    _reserveElements = numElements;
}
