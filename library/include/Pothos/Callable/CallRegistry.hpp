///
/// \file Callable/CallRegistry.hpp
///
/// CallRegistry is an interface for registering class methods.
///
/// \copyright
/// Copyright (c) 2014-2014 Josh Blum
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
    template <typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)());

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)() const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0) const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1) const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2) const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3) const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4) const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5) const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6) const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7) const);

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8));

    /*!
     * Register a class method with the given name.
     * Usage: this->registerCall(this, "foo", &MyClass::foo);
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType, typename InstanceType>
    void registerCall(InstanceType *instance, const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const);

    /*!
     * Register a bound call with the given name.
     * The first argument of the call should have the class instance bound.
     */
    virtual void registerCallable(const std::string &name, const Callable &call) = 0;
};

} //namespace Pothos
