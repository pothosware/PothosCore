//
// Util/RingDeque.hpp
//
// A templated double ended queue implemented on top of a vector.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <cstdlib> //size_t
#include <vector>
#include <cassert>

namespace Pothos {
namespace Util {

/*!
 * RingDeque is a template deque using an underlying vector.
 * The API of RingDeque is very similar to the std::deque<T>.
 * RingDeque is here because I wanted the efficiency of
 * boost::circular_buffer without the boost requirement.
 * The ring deque does not have a specific capacity limit.
 */
template <typename T>
class RingDeque
{
public:
    //! Construct a new ring deque
    RingDeque(void);

    //! Construct a new ring deque -- with space reservation
    RingDeque(const size_t capacity);

    //! Get a const ref at, where front == 0, back == size() - 1
    const T &operator[](const size_t offset) const;

    //! Get a ref at, where front == 0, back == size() - 1
    T &operator[](const size_t offset);

    //! Push an element onto the front of the queue
    void push_front(const T &elem);

    //! Push an element onto the front of the queue
    void push_front(T &&elem);

    //! Pop and element from the front of the queue
    void pop_front(void);

    //! Get a const reference to the front element
    const T &front(void) const;

    //! Get a reference to the front element
    T &front(void);

    //! Push an element onto the back of the queue
    void push_back(const T &elem);

    //! Push an element onto the back of the queue
    void push_back(T &&elem);

    //! Pop and element from the back of the queue
    void pop_back(void);

    //! Get a const reference to the back element
    const T &back(void) const;

    //! Get a reference to the back element
    T &back(void);

    //! Is the deque empty? -- no elements
    bool empty(void) const;

    //! Is the deque full? -- num elements == capacity
    bool full(void) const;

    //! How many elements are in the deque
    size_t size(void) const;

    //! How many elements can be stored?
    size_t capacity(void) const;

    //! Set the deque capacity if too small
    void set_capacity(const size_t capacity);

private:
    size_t _frontIndex;
    size_t _backIndex;
    size_t _numElements;
    std::vector<T> _container;
};

template <typename T>
RingDeque<T>::RingDeque(void):
    _frontIndex(0),
    _backIndex(0),
    _numElements(0),
    _container(1)
{
    return;
}

template <typename T>
RingDeque<T>::RingDeque(const size_t capacity):
    _frontIndex(0),
    _backIndex(capacity-1),
    _numElements(0),
    _container(capacity)
{
    assert(capacity > 0);
}

template <typename T>
const T &RingDeque<T>::operator[](const size_t offset) const
{
    return _container[(_frontIndex + offset) % _container.size()];
}

template <typename T>
T &RingDeque<T>::operator[](const size_t offset)
{
    return _container[(_frontIndex + offset) % _container.size()];
}

template <typename T>
void RingDeque<T>::push_front(const T &elem)
{
    assert(not this->full());
    _frontIndex = size_t(_frontIndex + _container.size() - 1) % _container.size();
    _container[_frontIndex] = elem;
    _numElements++;
}

template <typename T>
void RingDeque<T>::push_front(T &&elem)
{
    assert(not this->full());
    _frontIndex = size_t(_frontIndex + _container.size() - 1) % _container.size();
    _container[_frontIndex] = elem;
    _numElements++;
}

template <typename T>
void RingDeque<T>::pop_front(void)
{
    assert(not this->empty());
    assert(_frontIndex < _container.size());
    _container[_frontIndex] = T();
    _frontIndex = size_t(_frontIndex + 1) % _container.size();
    _numElements--;
}

template <typename T>
const T &RingDeque<T>::front(void) const
{
    assert(not this->empty());
    assert(_frontIndex < _container.size());
    return _container[_frontIndex];
}

template <typename T>
T &RingDeque<T>::front(void)
{
    assert(not this->empty());
    assert(_frontIndex < _container.size());
    return _container[_frontIndex];
}

template <typename T>
void RingDeque<T>::push_back(const T &elem)
{
    assert(not this->full());
    _backIndex = size_t(_backIndex + 1) % _container.size();
    _container[_backIndex] = elem;
    _numElements++;
}

template <typename T>
void RingDeque<T>::push_back(T &&elem)
{
    assert(not this->full());
    _backIndex = size_t(_backIndex + 1) % _container.size();
    _container[_backIndex] = elem;
    _numElements++;
}

template <typename T>
void RingDeque<T>::pop_back(void)
{
    assert(not this->empty());
    assert(_backIndex < _container.size());
    _container[_backIndex] = T();
    _backIndex = size_t(_backIndex + _container.size() - 1) % _container.size();
    _numElements--;
}

template <typename T>
const T &RingDeque<T>::back(void) const
{
    assert(not this->empty());
    assert(_backIndex < _container.size());
    return _container[_backIndex];
}

template <typename T>
T &RingDeque<T>::back(void)
{
    assert(not this->empty());
    assert(_backIndex < _container.size());
    return _container[_backIndex];
}

template <typename T>
bool RingDeque<T>::empty(void) const
{
    return _numElements == 0;
}

template <typename T>
bool RingDeque<T>::full(void) const
{
    return _numElements == _container.size();
}

template <typename T>
size_t RingDeque<T>::size(void) const
{
    return _numElements;
}

template <typename T>
size_t RingDeque<T>::capacity(void) const
{
    return _container.size();
}

template <typename T>
void RingDeque<T>::set_capacity(const size_t capacity)
{
    if (_numElements > capacity) return;
    std::vector<T> _newContainer(capacity);
    for (size_t i = 0; i < _numElements; i++)
    {
        _newContainer[i] = _container[(_frontIndex+i) % _container.size()];
    }
    _container = _newContainer;
    _frontIndex = 0;
    _backIndex = (_numElements + capacity - 1) % capacity;
}

} //namespace Util
} //namespace Pothos
