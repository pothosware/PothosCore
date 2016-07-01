///
/// \file Util/OrderedQueue.hpp
///
/// A templated queue that guarantee the ordering of pop
/// when elements are pushed into the queue with an index.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Util/RingDeque.hpp>
#include <cstdlib> //size_t
#include <utility> //std::forward
#include <vector>
#include <cassert>

namespace Pothos {
namespace Util {

/*!
 * OrderedQueue is a templated queue with an ordering index for push.
 * The ordering index is used to control the order at the queue front.
 */
template <typename T>
class OrderedQueue
{
public:
    //! Construct a size-zero ordered queue
    OrderedQueue(void);

    //! Construct a new ordered queue -- with space reservation
    OrderedQueue(const size_t capacity);

    /*!
     * Is this queue empty?
     * The queue is empty when the front element is not available.
     * Due to ordering, this queue can still be empty after push.
     */
    bool empty(void) const;

    /*!
     * Push an element into the queue.
     * \param elem the new element to push
     * \param index the order of the element
     */
    template <typename U>
    void push(U &&elem, const size_t index);

    /*!
     * Get access to the element at the front of the queue
     */
    const T &front(void) const;

    /*!
     * Get access to the element at the front of the queue
     */
    T &front(void);

    /*!
     * Pop an element from the front of the queue.
     */
    void pop(void);

    //! How many elements can be stored?
    size_t capacity(void) const;

private:
    size_t _indexToAck;
    std::vector<T> _pushedElems;
    std::vector<bool> _pushedElemsFlag;
    Pothos::Util::RingDeque<T> _readyElems;
};

template <typename T>
OrderedQueue<T>::OrderedQueue(void):
    _indexToAck(0)
{
    return;
}

template <typename T>
OrderedQueue<T>::OrderedQueue(const size_t capacity):
    _indexToAck(0),
    _pushedElems(capacity),
    _pushedElemsFlag(capacity, false),
    _readyElems(capacity)
{
    return;
}

template <typename T>
bool OrderedQueue<T>::empty(void) const
{
    return _readyElems.empty();
}

template <typename T>
template <typename U>
void OrderedQueue<T>::push(U &&elem, const size_t index)
{
    //store the element into its position
    assert(index < _pushedElems.size());
    _pushedElems[index] = std::forward<U>(elem);
    _pushedElemsFlag[index] = true;

    //look for pushed elements -- but in order
    while (_pushedElemsFlag[_indexToAck])
    {
        //move the element into the queue
        assert(not _readyElems.full());
        _readyElems.push_back(_pushedElems[_indexToAck]);
        _pushedElems[_indexToAck] = T(); //clear reference
        _pushedElemsFlag[_indexToAck] = false;

        //increment for the next pushed element
        if (++_indexToAck == _pushedElems.size()) _indexToAck = 0;
    }
}

template <typename T>
const T &OrderedQueue<T>::front(void) const
{
    return _readyElems.front();
}

template <typename T>
T &OrderedQueue<T>::front(void)
{
    return _readyElems.front();
}

template <typename T>
void OrderedQueue<T>::pop(void)
{
    _readyElems.pop_front();
}

template <typename T>
size_t OrderedQueue<T>::capacity(void) const
{
    return _readyElems.capacity();
}

} //namespace Util
} //namespace Pothos
