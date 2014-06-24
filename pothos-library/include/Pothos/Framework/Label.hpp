//
// Framework/Label.hpp
//
// Label and associated classes for decorating a stream of elements.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>

namespace Pothos {

/*!
 * A Label decorates a stream of information with meta-data.
 * The label's data is an Object with arbitrary contents.
 * The label's index indentifies an element in a stream.
 */
class POTHOS_API Label
{
public:
    //! Create an empty label with null data and zero index.
    Label(void);

    //! Create a label with specified data of type Object and index
    Label(const Object &data, const unsigned long long index);

    //! Create a label with specified data of ValueType and index
    template <typename ValueType>
    Label(ValueType &&data, const unsigned long long index);

    /*!
     * The data can be anything that can be held by Object.
     */
    Object data;

    /*!
     * The index specifies an absolute offset in a stream of elements.
     * To associate with the first element of a stream, index should be 0.
     * To associate with the Nth element of a stream, index should be N-1.
     */
    unsigned long long index;

    //! support for sorting Labels by index
    bool operator<(const Label &other) const;
};

//! Are these two labels equivalent? index and data must be equal
bool operator==(const Label &rhs, const Label &lhs);

/*!
 * LabelIteratorRange represents a sorted range of labels.
 * It has the standard iterator interface begin()/end().
 */
class POTHOS_API LabelIteratorRange
{
public:

    //! Const Label iterator type
    typedef const Label* const_iterator;

    //! Create an empty/invalid LabelIteratorRange
    LabelIteratorRange(void);

    //! Create a LabelIteratorRange from a begin() and end() iterator
    template <typename IterType>
    LabelIteratorRange(const IterType &begin, const IterType &end);

    //! Create a LabelIteratorRange from an existing range
    template <typename RangeType>
    LabelIteratorRange(const RangeType &range);

    //! Get the begining of the iterator range (inclusive)
    const_iterator begin(void) const;

    //! Get the end of the iterator range (exclusive)
    const_iterator end(void) const;

private:
    const_iterator _begin;
    const_iterator _end;
};

} //namespace Pothos

template <typename ValueType>
Pothos::Label::Label(ValueType &&data, const unsigned long long index):
    data(Object::make(std::forward<ValueType>(data))), index(index)
{
    return;
}

inline bool Pothos::operator==(const Label &rhs, const Label &lhs)
{
    return rhs.index == lhs.index and rhs.data == lhs.data;
}

inline bool Pothos::Label::operator<(const Label &other) const
{
    return this->index < other.index;
}

template <typename IterType>
Pothos::LabelIteratorRange::LabelIteratorRange(const IterType &begin, const IterType &end):
    _begin(&(*begin)), _end(&(*end))
{
    return;
}

template <typename RangeType>
Pothos::LabelIteratorRange::LabelIteratorRange(const RangeType &range):
    _begin(&(*range.begin())), _end(&(*range.end()))
{
    return;
}

inline Pothos::LabelIteratorRange::const_iterator Pothos::LabelIteratorRange::begin(void) const
{
    return _begin;
}

inline Pothos::LabelIteratorRange::const_iterator Pothos::LabelIteratorRange::end(void) const
{
    return _end;
}
