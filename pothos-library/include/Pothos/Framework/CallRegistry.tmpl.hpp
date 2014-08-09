//
// Framework/CallRegistry.hpp
//
// CallRegistry is an interface for registering class methods.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/Callable.hpp>
#include <string>

namespace Pothos {

/*!
 * CallRegistry is an interface for registering class methods.
 */
class POTHOS_API CallRegistry
{
public:

    //! Virtual destructor for subclassing
    virtual ~CallRegistry(void);

    #for $NARGS in range($MAX_ARGS)
    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <$expand('typename A%d', $NARGS), typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)($expand('A%d', $NARGS)));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <$expand('typename A%d', $NARGS), typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)($expand('A%d', $NARGS)) const);

    #end for
    /*!
     * Register a bound call with the given name.
     * The first argument of the call should have the class instance bound.
     */
    virtual void registerCallable(const std::string &name, const Callable &call) = 0;
};

} //namespace Pothos
