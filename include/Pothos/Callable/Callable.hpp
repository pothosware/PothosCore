///
/// \file Callable/Callable.hpp
///
/// Callable provides an opaque proxy for function or method calls.
///
/// \copyright
/// Copyright (c) 2013-2019 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/CallInterface.hpp>
#include <Pothos/Object/Object.hpp>
#include <vector>
#include <memory>

namespace Pothos {

//messy forward declares
namespace Detail {
struct CallableContainer;
} //namespace Detail

/*!
 * The Callable class binds a class method or function.
 * The method/function can be called through an opaque Object interface,
 * or through a templated call interface with an arbitrary number of args.
 */
class POTHOS_API Callable : public CallInterface
{
public:

    /*!
     * Create a null Callable.
     * Calling a null instance will throw CallableNullError.
     */
    Callable(void);

    /*!
     * Does the Callable hold a bound function?
     * \return true if callable holds a bound function
     */
    explicit operator bool(void) const;

    /*!
     * Call into the function/method with opaque input and return types.
     * For a void return type of call, the returned Object will be null.
     * \throws CallableNullError if the Callable is null
     * \throws CallableArgumentError for bad arguments in number or type
     * \param inputArgs an array of call arguments of type Object
     * \param numArgs the number of arguments in inputArgs
     * \return the return value of the call as type Object
     */
    Object opaqueCall(const Object *inputArgs, const size_t numArgs) const;

    /*!
     * Get the number of arguments for this call.
     * For methods, the class instance also counts
     * \return the number of arguments
     */
    size_t getNumArgs(void) const;

    /*!
     * Get the type info for a particular argument.
     * For the return type, use argNo = -1.
     * For argument 0 use argNo = 0, etc.
     * For methods, argNo = 0 is the class type.
     * \throws CallableArgumentError if the argNo is invalid
     * \return the type info for the argument
     */
    const std::type_info &type(const int argNo) const;

    /*!
     * Bind an argument to the given argument index - template version.
     * The user will not specify this argument at call time,
     * the bound argument will be used as a substitute.
     * The bind call does not throw.
     * \param val the value to put into the argument list
     * \param argNo the argument index to bind to
     * \return this Callable for operator chaining
     */
    template <typename ValueType>
    Callable &bind(ValueType &&val, const size_t argNo);

    /*!
     * Bind an argument to the given argument index - Object version.
     * The user will not specify this argument at call time,
     * the bound argument will be used as a substitute.
     * The bind call does not throw.
     * \param val the value to put into the argument list
     * \param argNo the argument index to bind to
     * \return this Callable for operator chaining
     */
    Callable &bind(Object &&val, const size_t argNo);

    /*!
     * Remove a binding at the given argument index.
     * The unbind call does not throw.
     * \param argNo the argument index to clear
     * \return this Callable for operator chaining
     */
    Callable &unbind(const size_t argNo);

    /*!
     * Get a string representation for this Callable.
     * The string holds the return type, and argument types.
     * \return the string with type names
     */
    std::string toString(void) const;

    //! Create a Callable for a class method with variable args
    template <typename ReturnType, typename ClassType, typename... ArgsType>
    Callable(ReturnType(ClassType::*fcn)(ArgsType...));

    //! Create a Callable for a const class method with variable args
    template <typename ReturnType, typename ClassType, typename... ArgsType>
    Callable(ReturnType(ClassType::*fcn)(ArgsType...) const);

    //! Create a Callable for a function with variable args
    template <typename ReturnType, typename... ArgsType>
    Callable(ReturnType(*fcn)(ArgsType...));

    //! Create a Callable for a std::function with variable args
    template <typename ReturnType, typename... ArgsType>
    Callable(const std::function<ReturnType(ArgsType...)> &fcn);

    /*!
     * Create a Callable for a class method with variable args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     * In this case, the entire pack must be specified.
     */
    template <typename ReturnType, typename ClassType, typename... ArgsType>
    static Callable make(ReturnType(ClassType::*fcn)(ArgsType...));

    /*!
     * Create a Callable for a const class method with variable args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     * In this case, the entire pack must be specified.
     */
    template <typename ReturnType, typename ClassType, typename... ArgsType>
    static Callable make(ReturnType(ClassType::*fcn)(ArgsType...) const);

    /*!
     * Create a Callable for a function with variable args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     * In this case, the entire pack must be specified.
     */
    template <typename ReturnType, typename... ArgsType>
    static Callable make(ReturnType(*fcn)(ArgsType...));

    /*!
     * Create a Callable for a std::function with variable args
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     * In this case, the entire pack must be specified.
     */
    template <typename ReturnType, typename... ArgsType>
    static Callable make(const std::function<ReturnType(ArgsType...)> &fcn);

    /*!
     * Create a Callable for a constructor with variable args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename... ArgsType>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with variable args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename... ArgsType>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with variable args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename... ArgsType>
    static Callable factoryShared(void);

private:
    std::vector<Object> _boundArgs;
    std::shared_ptr<Detail::CallableContainer> _impl;
    Callable(Detail::CallableContainer *impl);
    POTHOS_API friend bool operator==(const Callable &lhs, const Callable &rhs);
};

/*!
 * The equals operators checks if two Callable represent the same internal data.
 * The actual bound functions/methods cannot be checked for equality.
 * Two callables are only equal if they originated from the same construction.
 * \param lhs the left hand object of the comparison
 * \param rhs the right hand object of the comparison
 * \return true if the objects represent the same internal data
 */
POTHOS_API bool operator==(const Callable &lhs, const Callable &rhs);

} //namespace Pothos
