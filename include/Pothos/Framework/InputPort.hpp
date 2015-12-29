///
/// \file Framework/InputPort.hpp
///
/// This file provides an interface for a worker's input port.
///
/// \copyright
/// Copyright (c) 2014-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/Label.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/BufferAccumulator.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <Pothos/Util/SpinLock.hpp>
#include <string>

namespace Pothos {

class WorkerActor;
class OutputPort;

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

    //! Get a displayable name for this port.
    const std::string &alias(void) const;

    //! Set the displayable alias for this port.
    void setAlias(const std::string &alias);

    //! Get the data type information for this port.
    const DType &dtype(void) const;

    //! Get the domain information for this port
    const std::string &domain(void) const;

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
     * Get the total number of buffers posted to this port.
     * Note that this call tracks incoming buffer count,
     * and not total buffer consumption (which is harder).
     */
    unsigned long long totalBuffers(void) const;

    /*!
     * Get the total number of labels consumed from this port.
     * This count updates immediately upon calling removeLabel(),
     * and after after execution of work() and propagateLabels().
     */
    unsigned long long totalLabels(void) const;

    /*!
     * Get the total number of messages popped from this port.
     * The value returned by this method will be incremented
     * immediately upon calling popMessage().
     */
    unsigned long long totalMessages(void) const;

    //! Does the specified input port have an asynchronous message available?
    bool hasMessage(void);

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
     * The reserve size ensures that when sufficient resources are available,
     * the buffer will contain at least the specified number of elements.
     * By default, each input port has a reserve of zero elements,
     * which means that the input port's buffer may be any size,
     * including empty, depending upon the available resources.
     * Note that work() may still be called when the reserve is not met,
     * because the scheduler will only prevent work() from being called
     * when all ports fail to meet their respective reserve requirements.
     * \param numElements the number of elements to require
     */
    void setReserve(const size_t numElements);

    /*!
     * Is this port used for signal handling in a signals + slots paradigm?
     */
    bool isSlot(void) const;

    /*!
     * Push a buffer into the buffer queue of this input port.
     * This is a thread-safe call, it can be made from any context.
     * Use pushBuffer to preload an input port with elements, example:
     * a window-sized history of elements for a filter block,
     * or a preloaded number of elements for a feedback loop.
     */
    void pushBuffer(const BufferChunk &buffer);

    /*!
     * Push a label into the label storage of this input port.
     * This is a thread-safe call, it can be made from any context.
     * When using this call, first push the buffer with the
     * corresponding label index *before* pushing the label.
     */
    void pushLabel(const Label &label);

    /*!
     * Push a message into the message queue of this input port.
     * This is a thread-safe call, it can be made from any context.
     */
    void pushMessage(const Object &message);

    /*!
     * Clear all memory on this input port.
     * Clear buffers, labels, and messages.
     */
    void clear(void);

private:
    WorkerActor *_actor;

    //port configuration
    bool _isSlot;
    int _index;
    std::string _name;
    std::string _alias;
    DType _dtype;
    std::string _domain;

    //state set in pre-work
    BufferChunk _buffer;
    size_t _elements;
    LabelIteratorRange _labelIter;

    //port stats
    unsigned long long _totalElements;
    unsigned long long _totalBuffers;
    unsigned long long _totalLabels;
    unsigned long long _totalMessages;

    //state changes from work
    size_t _pendingElements;
    size_t _reserveElements;

    //counts work actions which we will use to establish activity
    size_t _workEvents;

    Util::SpinLock _asyncMessagesLock;
    Util::RingDeque<std::pair<Object, BufferChunk>> _asyncMessages;

    Util::SpinLock _slotCallsLock;
    Util::RingDeque<std::pair<Object, BufferChunk>> _slotCalls;

    std::vector<Label> _inlineMessages; //user api structure
    Util::RingDeque<Label> _inputInlineMessages; //shared structure

    Util::SpinLock _bufferAccumulatorLock;
    BufferAccumulator _bufferAccumulator;

    std::vector<OutputPort *> _subscribers;

    /////// async message interface /////////
    void asyncMessagesPush(const Object &message, const BufferChunk &token = BufferChunk::null());
    bool asyncMessagesEmpty(void);
    Object asyncMessagesPop(void);
    void asyncMessagesClear(void);

    /////// slot call interface /////////
    void slotCallsPush(const Object &args, const BufferChunk &token);
    bool slotCallsEmpty(void);
    Object slotCallsPop(void);
    void slotCallsClear(void);

    /////// inline message interface /////////
    void inlineMessagesPush(const Label &label);
    void inlineMessagesClear(void);

    /////// input buffer interface /////////
    void bufferAccumulatorFront(BufferChunk &);
    void bufferAccumulatorPush(const BufferChunk &buffer);
    void bufferAccumulatorPushNoLock(const BufferChunk &buffer);
    void bufferAccumulatorPop(const size_t numBytes);
    void bufferAccumulatorRequire(const size_t numBytes);
    void bufferAccumulatorClear(void);

    /////// combined label association push /////////
    void bufferLabelPush(
        const std::vector<Label> &postedLabels,
        const Util::RingDeque<BufferChunk> &postedBuffers);

    InputPort(void);
    InputPort(const InputPort &){} // non construction-copyable
    InputPort &operator=(const InputPort &){return *this;} // non copyable
    friend class WorkerActor;
    friend class OutputPort;
};

} //namespace Pothos
