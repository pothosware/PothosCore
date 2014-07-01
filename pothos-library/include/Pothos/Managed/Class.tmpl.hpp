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
     * Register a public base class of this managed class.
     * This adopts the inherited methods of the base class.
     */
    template <typename ClassType, typename BaseClassType>
    ManagedClass &registerBaseClass(void);

    /*!
     * Register a conversion function from this class to a base class.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     * \param toBase a callable that converts between class types
     */
    ManagedClass &registerToBaseClass(const Callable &toBase);

    #for $NARGS in range($MAX_ARGS)
    /*!
     * Register a constructor given class type and $NARGS argument types.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <typename ClassType, $expand('typename A%d', $NARGS)>
    ManagedClass &registerConstructor(void);

    /*!
     * Register a static method given class name and function pointer of $NARGS args.
     */
    template <$expand('typename A%d', $NARGS), typename ReturnType>
    ManagedClass &registerStaticMethod(const std::string &name, ReturnType(*method)($expand('A%d', $NARGS)));

    /*!
     * Register a method given class name and function pointer of $NARGS args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <$expand('typename A%d', $NARGS), typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)($expand('A%d', $NARGS)));

    /*!
     * Register a method given class name and function pointer of $NARGS args.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     */
    template <$expand('typename A%d', $NARGS), typename ReturnType, typename ClassType>
    ManagedClass &registerMethod(const std::string &name, ReturnType(ClassType::*method)($expand('A%d', $NARGS)) const);
    #end for

    /*!
     * Register an accessor and mutator method for the field.
     * The accessor and mutator are registered under the names "get:name" and "set:name".
     * Example usage: myManagedClass.registerField(&MyClass:myField);
     * The accessor method can be called on a class instance and takes no arguments.
     * The mutator method can be called on a class instance and takes the new value as the first argument.
     * \throws ManagedClassTypeError if the class type differs from the registered type
     * \param name the name of the field
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
     * Get a list of available converters to base classes
     * \return a list of converters as callables
     */
    const std::vector<Callable> &getBaseClassConverters(void) const;

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
