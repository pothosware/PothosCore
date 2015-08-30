///
/// \file Object/ObjectM.hpp
///
/// ObjectM provides a mutable subclass of Object.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
//We need to declare lots of copy constructors so the templated version is only called explicitly.
//However, the constructors cause the following warning on MSVC, which we disable below:
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning (disable:4521) // 'class' : multiple copy constructors specified
#endif //_MSC_VER

#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>

namespace Pothos {

/*!
 * ObjectM is a special mutable subclass of Object.
 *
 * When ObjectM is created, its internal data can be modified via extract().
 * ObjectM can be implicitly casted as Object to pass into read-only calls.
 * However, an Object instance cannot be converted into an ObjectM.
 *
 * For example: Users may create an ObjectM rather than Object to
 * modify its contents as part of a initialization routine.
 */
class POTHOS_API ObjectM : public Object
{
public:
    /*!
     * Create a null ObjectM.
     */
    explicit ObjectM(void);

    //! Copy constructor for ObjectM
    ObjectM(const ObjectM &obj);

    //! Move constructor for ObjectM
    ObjectM(ObjectM &&obj);

    //! Copy constructor for ObjectM
    ObjectM(ObjectM &obj);

    //! Move constructor for ObjectM
    ObjectM(const ObjectM &&obj);

    /*!
     * Create a new ObjectM from an arbitrary value.
     * \param value the data to store internally
     */
    template <typename ValueType>
    explicit ObjectM(ValueType &&value);

    //! Destructor for ObjectM
    virtual ~ObjectM(void);

    //! Copy assignment operator for ObjectM
    ObjectM &operator=(const ObjectM &rhs);

    //! Move assignment operator for ObjectM
    ObjectM &operator=(ObjectM &&rhs);

    /*!
     * Cast the internal data to an arbitrary type.
     * The requested cast type must exactly match the type().
     * \throws ObjectConvertError if object type != ValueType
     * \return a non-const reference to the internal data
     */
    template <typename ValueType>
    ValueType &extract(void) const;
};

} //namespace Pothos

#ifdef _MSC_VER
#pragma warning(pop)
#endif //_MSC_VER
