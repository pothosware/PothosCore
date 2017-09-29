///
/// \file Callable/CallInterface.hpp
///
/// CallInterface provides an method call abstraction interface.
///
/// \copyright
/// Copyright (c) 2013-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>

namespace Pothos {

/*!
 * CallInterface provides a set of templated call functions.
 * Derived classes overload opaqueCall() to handle the call.
 */
class POTHOS_API CallInterface
{
public:
    /*!
     * virtual destructor for inheritance
     */
    virtual ~CallInterface(void);

    //! Call a bound method/function with a return type and variable args
    template <typename ReturnType, typename... ArgsType>
    ReturnType call(ArgsType&&... args) const;

    //! Call a bound method/function with an Object return and variable args
    template <typename... ArgsType>
    Object call(ArgsType&&... args) const;

    /*!
     * Call a bound method/function with an Object return and variable args
     * \deprecated use call overload without return type
     */
    template <typename... ArgsType>
    POTHOS_DEPRECATED
    Object callObject(ArgsType&&... args) const;

    /*!
     * Call a bound method/function with a void return and variable args
     * \deprecated use call overload without return type
     */
    template <typename... ArgsType>
    POTHOS_DEPRECATED
    void callVoid(ArgsType&&... args) const;

protected:
    /*!
     * Call into the function/method with opaque input and return types.
     * For a void return type of call, the returned Object will be null.
     * \param inputArgs an array of call arguments of type Object
     * \param numArgs the number of arguments in inputArgs
     * \return the return value of the call as type Object
     */
    virtual Object opaqueCall(const Object *inputArgs, const size_t numArgs) const = 0;
};

} //namespace Pothos
