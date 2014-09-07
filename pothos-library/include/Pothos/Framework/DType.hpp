///
/// \file Framework/DType.hpp
///
/// This file contains the definition for the DType object.
///
/// \copyright
/// Copyright (c) 2014-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <typeinfo>
#include <vector>
#include <string>

namespace Pothos {

/*!
 * DType provides meta-information about a data type.
 * A DType consists of a name that identifies the type,
 * a shape that describes the dimensionality of the type.
 *
 * DType will recognize the names for the most common types like:
 * primitive integers, fixed width types, floating point types, complex...
 * If the name is not regonized, the user must specify the size in bytes.
 *
 * Recognized name strings:
 *  - un/signed char/int/short/long/long long
 *  - u/int8, u/int16, u/int32, u/int64
 *  - float, double, float32, float64
 *  - complex64, complex128
 *
 * The shape attribute describes the shape or dimensionality of the type.
 * An empty shape means that the DType represents a single element.
 * To represent a vector of size N elements, shape should be [N].
 * To represent a matrix of size NxM elements, shape should be [N, M].
 *
 * The size of a DType represents the size of an element times its shape.
 * Size in bytes = element size * shape[0] * shape[1]... and so on.
 */
class POTHOS_API DType
{
public:

    //! Typedef for the dimensionality represenation
    typedef std::vector<size_t> Shape;

    //! Create an empty DType with blank name, empty shape, and size 0
    DType(void);

    /*!
     * Create a DType from only a name.
     * The size will be inferred for the most common names.
     * \throw DTypeUnknownError when the name is not known
     * \param name the name identfier of a known DType
     */
    DType(const char *name);

    /*!
     * Create a DType from only a name.
     * The size will be inferred for the most common names.
     * \throw DTypeUnknownError when the name is not known
     * \param name the name identfier of a known DType
     */
    DType(const std::string &name);

    /*!
     * Create a DType from a name and shape.
     * The size will be inferred for the most common names,
     * times the dimensionality of the shape.
     * \throw DTypeUnknownError when the name is not known
     * \param name the name identfier of a known DType
     * \param shape the shape dimensions of this DType
     */
    DType(const std::string &name, const Shape &shape);

    /*!
     * Create a DType from unknown name identifier, specifying the size.
     * The size should be the size in bytes of a single element.
     * The resulting size of this DType will be the elemSize * shape.
     * \param name the name identfier of a known DType
     * \param elemSize the size of a single element in bytes
     * \param shape the shape dimensions of this DType
     */
    DType(const std::string &name, const size_t elemSize, const Shape &shape = Shape());

    /*!
     * Create a DType from a type_info identifier.
     * The size and name of the type are inferred from the type info.
     * The resulting size of this DType will be the elemSize * shape.
     * \throw DTypeUnknownError when the type is not known
     * \param type a recognized type info object
     * \param shape the shape dimensions of this DType
     */
    DType(const std::type_info &type, const Shape &shape = Shape());

    /*!
     * Get a name that describes this DType.
     * If the name specified at construction was a recognized alias
     * for the same type under a different name, then the well-known
     * alias name will be used. This is typically a fixed-width
     * version of the name like int32, uint16, complex64, etc...
     */
    const std::string &name(void) const;

    //! Get the shape of this DType
    const Shape &shape(void) const;

    //! Get the size of a single element in bytes
    size_t elemSize(void) const;

    //! Get the size of this DType in bytes
    size_t size(void) const;

    //! Create a printable string representation
    std::string toString(void) const;

    //! Serialization support
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

private:
    std::string _name;
    Shape _shape;
    size_t _elemSize;
    size_t _size;
};

//! Equality operator for DType (all attributes must match for equality)
POTHOS_API bool operator==(const DType &lhs, const DType &rhs);

} //namespace Pothos

inline const std::string &Pothos::DType::name(void) const
{
    return _name;
}

inline const Pothos::DType::Shape &Pothos::DType::shape(void) const
{
    return _shape;
}

inline size_t Pothos::DType::elemSize(void) const
{
    return _elemSize;
}

inline size_t Pothos::DType::size(void) const
{
    return _size;
}
