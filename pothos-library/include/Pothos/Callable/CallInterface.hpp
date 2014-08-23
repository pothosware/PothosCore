//
// Callable/CallInterface.hpp
//
// CallInterface provides an method call abstraction interface.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

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

    /*!
     * Call into the function/method with opaque input and return types.
     * For a void return type of call, the returned Object will be null.
     * \param inputArgs an array of call arguments of type Object
     * \param numArgs the number of arguments in inputArgs
     * \return the return value of the call as type Object
     */
    virtual Object opaqueCall(const Object *inputArgs, const size_t numArgs) const = 0;

    //! Call a bound method/function with a return type and 0 args
    template <typename ReturnType>
    ReturnType call() const;

    //! Call a bound method/function with an Object return and 0 args
    inline
    Object callObject() const;

    //! Call a bound method/function with a void return and 0 args
    inline
    void callVoid() const;
    //! Call a bound method/function with a return type and 1 args
    template <typename ReturnType, typename A0>
    ReturnType call(A0 &&a0) const;

    //! Call a bound method/function with an Object return and 1 args
    template <typename A0>
    Object callObject(A0 &&a0) const;

    //! Call a bound method/function with a void return and 1 args
    template <typename A0>
    void callVoid(A0 &&a0) const;
    //! Call a bound method/function with a return type and 2 args
    template <typename ReturnType, typename A0, typename A1>
    ReturnType call(A0 &&a0, A1 &&a1) const;

    //! Call a bound method/function with an Object return and 2 args
    template <typename A0, typename A1>
    Object callObject(A0 &&a0, A1 &&a1) const;

    //! Call a bound method/function with a void return and 2 args
    template <typename A0, typename A1>
    void callVoid(A0 &&a0, A1 &&a1) const;
    //! Call a bound method/function with a return type and 3 args
    template <typename ReturnType, typename A0, typename A1, typename A2>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2) const;

    //! Call a bound method/function with an Object return and 3 args
    template <typename A0, typename A1, typename A2>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2) const;

    //! Call a bound method/function with a void return and 3 args
    template <typename A0, typename A1, typename A2>
    void callVoid(A0 &&a0, A1 &&a1, A2 &&a2) const;
    //! Call a bound method/function with a return type and 4 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const;

    //! Call a bound method/function with an Object return and 4 args
    template <typename A0, typename A1, typename A2, typename A3>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const;

    //! Call a bound method/function with a void return and 4 args
    template <typename A0, typename A1, typename A2, typename A3>
    void callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const;
    //! Call a bound method/function with a return type and 5 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const;

    //! Call a bound method/function with an Object return and 5 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const;

    //! Call a bound method/function with a void return and 5 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    void callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const;
    //! Call a bound method/function with a return type and 6 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const;

    //! Call a bound method/function with an Object return and 6 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const;

    //! Call a bound method/function with a void return and 6 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    void callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const;
    //! Call a bound method/function with a return type and 7 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const;

    //! Call a bound method/function with an Object return and 7 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const;

    //! Call a bound method/function with a void return and 7 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    void callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const;
    //! Call a bound method/function with a return type and 8 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const;

    //! Call a bound method/function with an Object return and 8 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const;

    //! Call a bound method/function with a void return and 8 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    void callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const;
    //! Call a bound method/function with a return type and 9 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const;

    //! Call a bound method/function with an Object return and 9 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const;

    //! Call a bound method/function with a void return and 9 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    void callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const;
};

} //namespace Pothos

inline Pothos::Object Pothos::CallInterface::callObject(void) const
{
    return this->opaqueCall(nullptr, 0);
}

inline void Pothos::CallInterface::callVoid(void) const
{
    this->callObject();
}
