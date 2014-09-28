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
#include <string>

namespace Pothos {

/*!
 * DType provides meta-information about a data type.
 * A DType consists of a primitive element type and dimensionality.
 * The dimensionality is a simple number that allows a DType
 * to represent an integer number of primitive element per object.
 *
 * DType will recognize the alias names for the most common types like:
 * primitive integers, fixed width types, floating point types, complex...
 *
 * Recognized name strings:
 *  - un/signed char/int/short/long/long long
 *  - u/int8, u/int16, u/int32, u/int64
 *  - float, double, float32, float64
 *  - complex_[known_type]
 *  - complex64, complex128
 *
 * Special name strings:
 *  - empty string for an unspecified size-zero data type
 *  - "custom" for an unspecified dimensionality-size data type
 */
class POTHOS_API DType
{
public:

    //! Create an empty DType with blank name, empty shape, and size 0
    DType(void);

    /*!
     * Create a DType from only a markup string (char * overload).
     * \throw DTypeUnknownError when the name is not known
     * \param markup the name identfier of a known DType
     */
    DType(const char *markup);

    /*!
     * Create a DType from only a markup string.
     * The markup name is a type alias (like float32),
     * with optional comma separated dimensionality.
     * \throw DTypeUnknownError when the name is not known
     * \param markup the name identfier of a known DType
     */
    DType(const std::string &markup);

    /*!
     * Create a DType from a type alias and dimensionality.
     * \throw DTypeUnknownError when the name is not known
     * \param alias a string identfier for a known data type
     * \param dimension the number of elements per type
     */
    DType(const std::string &alias, const size_t dimension);

    /*!
     * Create a DType from a type_info identifier and optional dimensionality.
     * The size and type enum are inferred from the type info.
     * \throw DTypeUnknownError when the type is not known
     * \param type a recognized type info object
     * \param dimension the number of elements per type
     */
    DType(const std::type_info &type, const size_t dimension = 1);

    /*!
     * Get a name that describes an element.
     * Example: int32, uint16, complex_float32, etc...
     */
    const std::string &name(void) const;
    
    //! Get the element type descriptor
    size_t elemType(void) const;

    //! Get the size of a single element in bytes
    size_t elemSize(void) const;

    //! Get the dimensionality of this type
    size_t dimension(void) const;

    //! Get the size of this DType in bytes
    size_t size(void) const;

    //! Create a printable string representation
    std::string toString(void) const;

    /*!
     * Does this dtype specify a non-empty type?
     * \return false for default constructor/empty string
     */
    pothos_explicit operator bool(void) const;

    /*!
     * Does this dtype represent a custom type?
     * \return true when the type was constructed with "custom"
     */
    bool isCustom(void) const;

    /*!
     * Does this dtype represent a floating point data type?
     * \return true for floats, doubles, and complex floats, doubles
     */
    bool isFloat(void) const;

    /*!
     * Does this dtype represent an integer data type?
     * \return true for fixed width integer types and complex integer types
     */
    bool isInteger(void) const;

    /*!
     * Does this dtype represent a signed data type?
     * \return true for signed integer, complex signed integer types.
     */
    bool isSigned(void) const;

    /*!
     * Does this dtype represent a complex data type?
     * \return true when the type is std::complex<primitive type>
     */
    bool isComplex(void) const;

    //! Serialization support
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

private:
    size_t _elemType;
    size_t _elemSize;
    size_t _dimension;
};

//! Equality operator for DType (all attributes must match for equality)
POTHOS_API bool operator==(const DType &lhs, const DType &rhs);

} //namespace Pothos

inline bool Pothos::operator==(const DType &lhs, const DType &rhs)
{
    return lhs.elemType() == rhs.elemType() and
        lhs.elemSize() == rhs.elemSize() and
        lhs.dimension() == rhs.dimension();
}

inline Pothos::DType::DType(void):
    _elemType(0),
    _elemSize(1),
    _dimension(1)
{
    return;
}

inline size_t Pothos::DType::elemType(void) const
{
    return _elemType;
}

inline size_t Pothos::DType::elemSize(void) const
{
    return _elemSize;
}

inline size_t Pothos::DType::dimension(void) const
{
    return _dimension;
}

inline size_t Pothos::DType::size(void) const
{
    return _elemSize*_dimension;
}

inline Pothos::DType::operator bool(void) const
{
    return _elemType != 0;
}
