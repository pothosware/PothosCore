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

    #for $NARGS in range($MAX_ARGS)
    //! Call a bound method/function with a return type and $NARGS args
    template <typename ReturnType, $expand('typename A%d', $NARGS)>
    ReturnType call($expand('A%d &&a%d', $NARGS)) const;

    //! Call a bound method/function with an Object return and $NARGS args
    template <$expand('typename A%d', $NARGS)>
    Object callObject($expand('A%d &&a%d', $NARGS)) const;

    //! Call a bound method/function with a void return and $NARGS args
    template <$expand('typename A%d', $NARGS)>
    void callVoid($expand('A%d &&a%d', $NARGS)) const;

    #end for
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
