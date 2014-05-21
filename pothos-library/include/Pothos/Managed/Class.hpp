//
// Managed/Class.hpp
//
// Interface definition for a ManagedClass.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/Callable.hpp>
#include <string>
#include <memory>
#include <typeinfo>

namespace Pothos {

/*!
 * ManagedClass represents registered constructors and methods of a class.
 * A ManagedClass makes calls available to users via the Callable interface.
 */
class POTHOS_API ManagedClass
{
public:

    /*!
     * Lookup a managed class based on typeinfo for the class.
     */
    static ManagedClass lookup(const std::type_info &type);

    //! Create a new empty ManagedClass
    ManagedClass(void);

    /*!
     * Register information and conversions about the ClassType.
     * This routine is automatically called during
     * constructor and method registration calls.
     */
    template <typename ClassType>
    ManagedClass &registerClass(void);

    /*!
     * Register a constructor given class type and 0 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 0 args.
     */
    template <typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)());

    /*!
     * Register a method given class name and function pointer of 0 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)());

    /*!
     * Register a method given class name and function pointer of 0 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)() const);
    /*!
     * Register a constructor given class type and 1 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 1 args.
     */
    template <typename A0, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0));

    /*!
     * Register a method given class name and function pointer of 1 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0));

    /*!
     * Register a method given class name and function pointer of 1 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0) const);
    /*!
     * Register a constructor given class type and 2 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0, typename A1>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 2 args.
     */
    template <typename A0, typename A1, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1));

    /*!
     * Register a method given class name and function pointer of 2 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1));

    /*!
     * Register a method given class name and function pointer of 2 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1) const);
    /*!
     * Register a constructor given class type and 3 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0, typename A1, typename A2>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 3 args.
     */
    template <typename A0, typename A1, typename A2, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2));

    /*!
     * Register a method given class name and function pointer of 3 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2));

    /*!
     * Register a method given class name and function pointer of 3 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2) const);
    /*!
     * Register a constructor given class type and 4 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 4 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3));

    /*!
     * Register a method given class name and function pointer of 4 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3));

    /*!
     * Register a method given class name and function pointer of 4 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3) const);
    /*!
     * Register a constructor given class type and 5 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 5 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4));

    /*!
     * Register a method given class name and function pointer of 5 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4));

    /*!
     * Register a method given class name and function pointer of 5 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4) const);
    /*!
     * Register a constructor given class type and 6 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 6 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4, A5));

    /*!
     * Register a method given class name and function pointer of 6 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5));

    /*!
     * Register a method given class name and function pointer of 6 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5) const);
    /*!
     * Register a constructor given class type and 7 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 7 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4, A5, A6));

    /*!
     * Register a method given class name and function pointer of 7 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6));

    /*!
     * Register a method given class name and function pointer of 7 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6) const);
    /*!
     * Register a constructor given class type and 8 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 8 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4, A5, A6, A7));

    /*!
     * Register a method given class name and function pointer of 8 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7));

    /*!
     * Register a method given class name and function pointer of 8 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7) const);
    /*!
     * Register a constructor given class type and 9 argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of 9 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8));

    /*!
     * Register a method given class name and function pointer of 9 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8));

    /*!
     * Register a method given class name and function pointer of 9 args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const);

    /*!
     * Register an accessor and mutator method for the field.
     * Example usage: myManagedClass.registerField(&MyClass:myField);
     * The accessor method can be called on a class instance and takes no arguments.
     * The mutator method can be called on a class instance and takes the new value as the first argument.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     * \param name the name of the field (and registered methods)
     * \param member a pointer to a data member of Class
     */
    template <typename ClassType, typename ValueType>
    ManagedClass &registerField(const std::string &name, ValueType ClassType::*member);

    /*!
     * Register a conversion function from reference to a ref wrapper.
     * Called automatically when a constructor is registered via the templated interface.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    ManagedClass &registerReferenceToWrapper(const Callable &toPointer);

    /*!
     * Register a conversion function from pointer to a ref wrapper.
     * Called automatically when a constructor is registered via the templated interface.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    ManagedClass &registerPointerToWrapper(const Callable &toPointer);

    /*!
     * Register a conversion function from shared pointer to ref wrapper.
     * Called automatically when a constructor is registered via the templated interface.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    ManagedClass &registerSharedToWrapper(const Callable &toPointer);

    /*!
     * Register a constructor via a callable.
     * This method is called by the templated method of the same name.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    ManagedClass &registerConstructor(const Callable &constructor);

    /*!
     * Register a static method via a callable.
     * This method is called by the templated method of the same name.
     */
    ManagedClass &registerStaticMethod(const std::string &name, const Callable &method);

    /*!
     * Register a method via a callable.
     * This method is called by the templated method of the same name.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    ManagedClass &registerMethod(const std::string &name, const Callable &method);

    /*!
     * Register a callable that takes generic arguments (Object *args, size_t num).
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    ManagedClass &registerOpaqueConstructor(const Callable &constructor);

    /*!
     * Template version of registerOpaqueConstructor for user's convenience.
     */
    template <typename ClassType>
    ManagedClass &registerOpaqueConstructor(void);

    /*!
     * Register a static method with the following signature:
     * Object staticMethod(Object *args, size_t num).
     */
    ManagedClass &registerOpaqueStaticMethod(const std::string &name, const Callable &method);

    /*!
     * Register a static method with the following signature:
     * Object staticMethod(std::string name, Object *args, size_t num).
     */
    ManagedClass &registerWildcardStaticMethod(const Callable &method);

    /*!
     * Register a static method with the following signature:
     * Object method(instance, Object *args, size_t num).
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    ManagedClass &registerOpaqueMethod(const std::string &name, const Callable &method);

    /*!
     * Register a static method with the following signature:
     * Object method(instance, std::string name, Object *args, size_t num).
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    ManagedClass &registerWildcardMethod(const Callable &method);

    /*!
     * Commit this registration into the plugin tree.
     * The actual registration will be stored to /managed/classPath.
     * An example class path might be "MyNamespace/MyClass".
     * \throws PluginPathError if the classPath is invalid
     * \param classPath the namespaces and class name
     */
    ManagedClass &commit(const std::string &classPath);

    /*!
     * Get the type of the class represented.
     * This is the type created by the constructors,
     * and the type taken as the class by the bound methods.
     * \throws ManagedClassTypeError if no constructors registered
     */
    const std::type_info &type(void) const;

    /*!
     * Get the pointer version of the class type represented.
     * \throws ManagedClassTypeError if no constructors registered
     */
    const std::type_info &pointerType(void) const;

    /*!
     * Get the shared pointer version of the class type represented.
     * \throws ManagedClassTypeError if no constructors registered
     */
    const std::type_info &sharedType(void) const;

    /*!
     * Get a callable that converts a reference into a ref wrapper.
     */
    const Callable &getReferenceToWrapper(void) const;

    /*!
     * Get a callable that converts a pointer into a ref wrapper.
     */
    const Callable &getPointerToWrapper(void) const;

    /*!
     * Get a callable that converts a shared pointer into a ref wrapper.
     */
    const Callable &getSharedToWrapper(void) const;

    /*!
     * Get a list of available constructors.
     * \return a list of constructors as callables
     */
    const std::vector<Callable> &getConstructors(void) const;

    /*!
     * Get a list of available static methods for the given method name.
     * \throws ManagedClassNameError if the name does not exist in the registry
     * \param name the name of the static method to look for
     * \return a list of static methods as callables
     */
    const std::vector<Callable> &getStaticMethods(const std::string &name) const;

    /*!
     * Get a list of available methods for the given method name.
     * \throws ManagedClassNameError if the name does not exist in the registry
     * \param name the name of the method to look for
     * \return a list of methods as callables
     */
    const std::vector<Callable> &getMethods(const std::string &name) const;

    /*!
     * Get the opaque constructor.
     * The opaque contructor takes (Object *args, size_t num).
     * \return a callable object for a constructor
     */
    const Callable &getOpaqueConstructor(void) const;

    /*!
     * Get the opaque static methods for the given method name.
     * The opaque static method takes (Object *args, size_t num).
     * \throws ManagedClassNameError if the name does not exist in the registry
     * \param name the name of the static method to look for
     * \return a callable object for a static method
     */
    const Callable &getOpaqueStaticMethod(const std::string &name) const;

    /*!
     * Get the wildcard opaque static method.
     * This is a catch-all for static methods when there is not a name match.
     * The opaque wildcard static method takes (std::string name, Object *args, size_t num).
     * \return a callable object for a static method
     */
    const Callable &getWildcardStaticMethod(void) const;

    /*!
     * Get the opaque method for the given method name.
     * The opaque method takes (Object *args, size_t num).
     * \throws ManagedClassNameError if the name does not exist in the registry
     * \param name the name of the method to look for
     * \return a callable object for a method
     */
    const Callable &getOpaqueMethod(const std::string &name) const;

    /*!
     * Get the wildcard static method.
     * This is a catch-all for methods when there is not a name match.
     * The opaque wildcard method takes (std::string name, Object *args, size_t num).
     * \return a callable object for a method
     */
    const Callable &getWildcardMethod(void) const;

private:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} //namespace Pothos
