//
// Callable/Callable.hpp
//
// Callable provides an opaque proxy for function or method calls.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
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
class POTHOS_API Callable
{
public:

    /*!
     * Create a null Callable.
     * Calling a null instance will throw CallableNullError.
     */
    Callable(void);

    /*!
     * Is the Callable null?
     * \return true if callable does not hold a function
     */
    bool null(void) const;

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

    //! Create a Callable for a class method with 0 args
    template <typename ReturnType, typename ClassType>
    Callable(ReturnType(ClassType::*fcn)());

    //! Create a Callable for a const class method with 0 args
    template <typename ReturnType, typename ClassType>
    Callable(ReturnType(ClassType::*fcn)() const);

    //! Create a Callable for a function with 0 args
    template <typename ReturnType>
    Callable(ReturnType(*fcn)());

    /*!
     * Create a Callable for a class method with 0 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)());

    /*!
     * Create a Callable for a const class method with 0 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)() const);

    /*!
     * Create a Callable for a function with 0 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename ReturnType>
    static Callable make(ReturnType(*fcn)());

    /*!
     * Create a Callable for a constructor with 0 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 0 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 0 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 0 args
    template <typename ReturnType>
    ReturnType call() const;

    //! Call a bound method/function with an Object return and 0 args
    inline
    Object callObject() const;

    //! Call a bound method/function with a void return and 0 args
    inline
    void call() const;
    //! Create a Callable for a class method with 1 args
    template <typename ReturnType, typename ClassType, typename A0>
    Callable(ReturnType(ClassType::*fcn)(A0));

    //! Create a Callable for a const class method with 1 args
    template <typename ReturnType, typename ClassType, typename A0>
    Callable(ReturnType(ClassType::*fcn)(A0) const);

    //! Create a Callable for a function with 1 args
    template <typename ReturnType, typename A0>
    Callable(ReturnType(*fcn)(A0));

    /*!
     * Create a Callable for a class method with 1 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0));

    /*!
     * Create a Callable for a const class method with 1 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0) const);

    /*!
     * Create a Callable for a function with 1 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0));

    /*!
     * Create a Callable for a constructor with 1 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 1 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 1 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 1 args
    template <typename ReturnType, typename A0>
    ReturnType call(A0 &&a0) const;

    //! Call a bound method/function with an Object return and 1 args
    template <typename A0>
    Object callObject(A0 &&a0) const;

    //! Call a bound method/function with a void return and 1 args
    template <typename A0>
    void call(A0 &&a0) const;
    //! Create a Callable for a class method with 2 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1>
    Callable(ReturnType(ClassType::*fcn)(A0, A1));

    //! Create a Callable for a const class method with 2 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1>
    Callable(ReturnType(ClassType::*fcn)(A0, A1) const);

    //! Create a Callable for a function with 2 args
    template <typename ReturnType, typename A0, typename A1>
    Callable(ReturnType(*fcn)(A0, A1));

    /*!
     * Create a Callable for a class method with 2 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1));

    /*!
     * Create a Callable for a const class method with 2 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1) const);

    /*!
     * Create a Callable for a function with 2 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0, A1));

    /*!
     * Create a Callable for a constructor with 2 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 2 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 2 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 2 args
    template <typename ReturnType, typename A0, typename A1>
    ReturnType call(A0 &&a0, A1 &&a1) const;

    //! Call a bound method/function with an Object return and 2 args
    template <typename A0, typename A1>
    Object callObject(A0 &&a0, A1 &&a1) const;

    //! Call a bound method/function with a void return and 2 args
    template <typename A0, typename A1>
    void call(A0 &&a0, A1 &&a1) const;
    //! Create a Callable for a class method with 3 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2));

    //! Create a Callable for a const class method with 3 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2) const);

    //! Create a Callable for a function with 3 args
    template <typename ReturnType, typename A0, typename A1, typename A2>
    Callable(ReturnType(*fcn)(A0, A1, A2));

    /*!
     * Create a Callable for a class method with 3 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2));

    /*!
     * Create a Callable for a const class method with 3 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2) const);

    /*!
     * Create a Callable for a function with 3 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0, A1, A2));

    /*!
     * Create a Callable for a constructor with 3 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 3 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 3 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 3 args
    template <typename ReturnType, typename A0, typename A1, typename A2>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2) const;

    //! Call a bound method/function with an Object return and 3 args
    template <typename A0, typename A1, typename A2>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2) const;

    //! Call a bound method/function with a void return and 3 args
    template <typename A0, typename A1, typename A2>
    void call(A0 &&a0, A1 &&a1, A2 &&a2) const;
    //! Create a Callable for a class method with 4 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3));

    //! Create a Callable for a const class method with 4 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3) const);

    //! Create a Callable for a function with 4 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3>
    Callable(ReturnType(*fcn)(A0, A1, A2, A3));

    /*!
     * Create a Callable for a class method with 4 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3));

    /*!
     * Create a Callable for a const class method with 4 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3) const);

    /*!
     * Create a Callable for a function with 4 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0, A1, A2, A3));

    /*!
     * Create a Callable for a constructor with 4 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 4 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 4 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 4 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const;

    //! Call a bound method/function with an Object return and 4 args
    template <typename A0, typename A1, typename A2, typename A3>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const;

    //! Call a bound method/function with a void return and 4 args
    template <typename A0, typename A1, typename A2, typename A3>
    void call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const;
    //! Create a Callable for a class method with 5 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4));

    //! Create a Callable for a const class method with 5 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4) const);

    //! Create a Callable for a function with 5 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4>
    Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4));

    /*!
     * Create a Callable for a class method with 5 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4));

    /*!
     * Create a Callable for a const class method with 5 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4) const);

    /*!
     * Create a Callable for a function with 5 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0, A1, A2, A3, A4));

    /*!
     * Create a Callable for a constructor with 5 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 5 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 5 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 5 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const;

    //! Call a bound method/function with an Object return and 5 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const;

    //! Call a bound method/function with a void return and 5 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    void call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const;
    //! Create a Callable for a class method with 6 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5));

    //! Create a Callable for a const class method with 6 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5) const);

    //! Create a Callable for a function with 6 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5));

    /*!
     * Create a Callable for a class method with 6 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5));

    /*!
     * Create a Callable for a const class method with 6 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5) const);

    /*!
     * Create a Callable for a function with 6 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5));

    /*!
     * Create a Callable for a constructor with 6 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 6 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 6 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 6 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const;

    //! Call a bound method/function with an Object return and 6 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const;

    //! Call a bound method/function with a void return and 6 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    void call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const;
    //! Create a Callable for a class method with 7 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6));

    //! Create a Callable for a const class method with 7 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6) const);

    //! Create a Callable for a function with 7 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6));

    /*!
     * Create a Callable for a class method with 7 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6));

    /*!
     * Create a Callable for a const class method with 7 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6) const);

    /*!
     * Create a Callable for a function with 7 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6));

    /*!
     * Create a Callable for a constructor with 7 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 7 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 7 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 7 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const;

    //! Call a bound method/function with an Object return and 7 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const;

    //! Call a bound method/function with a void return and 7 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    void call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const;
    //! Create a Callable for a class method with 8 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7));

    //! Create a Callable for a const class method with 8 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7) const);

    //! Create a Callable for a function with 8 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6, A7));

    /*!
     * Create a Callable for a class method with 8 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7));

    /*!
     * Create a Callable for a const class method with 8 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7) const);

    /*!
     * Create a Callable for a function with 8 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6, A7));

    /*!
     * Create a Callable for a constructor with 8 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 8 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 8 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 8 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const;

    //! Call a bound method/function with an Object return and 8 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const;

    //! Call a bound method/function with a void return and 8 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    void call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const;
    //! Create a Callable for a class method with 9 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8));

    //! Create a Callable for a const class method with 9 args
    template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const);

    //! Create a Callable for a function with 9 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8));

    /*!
     * Create a Callable for a class method with 9 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8));

    /*!
     * Create a Callable for a const class method with 9 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType>
    static Callable make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const);

    /*!
     * Create a Callable for a function with 9 args.
     * Use make to specify explicit template arguments
     * to differentiate overloads with the same name.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType>
    static Callable make(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8));

    /*!
     * Create a Callable for a constructor with 9 args.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    static Callable factory(void);

    /*!
     * Create a Callable for a constructor with 9 args.
     * The callable return type is a pointer to ClassType*.
     * The user is responsible for managing the memory.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    static Callable factoryNew(void);

    /*!
     * Create a Callable for a constructor with 9 args.
     * The callable return type is a std::shared_ptr<ClassType>.
     * Template arguments must be explicitly specified.
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    static Callable factoryShared(void);

    //! Call a bound method/function with a return type and 9 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    ReturnType call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const;

    //! Call a bound method/function with an Object return and 9 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    Object callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const;

    //! Call a bound method/function with a void return and 9 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    void call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const;

private:
    std::vector<Object> _boundArgs;
    std::shared_ptr<Detail::CallableContainer> _impl;
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

inline Pothos::Object Pothos::Callable::callObject(void) const
{
    return this->opaqueCall(nullptr, 0);
}

inline void Pothos::Callable::call(void) const
{
    this->callObject();
}
