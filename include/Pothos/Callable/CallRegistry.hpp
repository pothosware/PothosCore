///
/// \file Callable/CallRegistry.hpp
///
/// CallRegistry is an interface for registering class methods.
///
/// \copyright
/// Copyright (c) 2014-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

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

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename... ArgsType, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(ArgsType...));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename... ArgsType, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(ArgsType...) const);

    /*!
     * Register a bound call with the given name.
     * The first argument of the call should have the class instance bound.
     */
    virtual void registerCallable(const std::string &name, const Callable &call) = 0;
};

} //namespace Pothos
