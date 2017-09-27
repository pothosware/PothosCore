///
/// \file Proxy/Proxy.hpp
///
/// Definitions for the Proxy wrapper class.
///
/// \copyright
/// Copyright (c) 2013-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>
#include <memory>
#include <string>

namespace Pothos {

class ProxyEnvironment;
class ProxyHandle;

/*!
 * The Proxy is a wrapper class for making calls in a ProxyEnvironment.
 * Proxys are created by the Environment and by using Proxy call().
 * The Proxy methods are simply just templated convenience methods
 * that take any argument type provided and handle the automatic conversions.
 */
class POTHOS_API Proxy
{
public:

    /*!
     * Create a null Proxy.
     */
    Proxy(void);

    /*!
     * Create a Proxy from a handle.
     * This constructor will typically be called by the implementation.
     * \param handle a ProxyHandle shared pointer created by an environment
     */
    Proxy(const std::shared_ptr<ProxyHandle> &handle);

    /*!
     * Create a Proxy from a handle.
     * The Proxy is responsible for deletion of the pointer.
     * This constructor will typically be called by the implementation.
     * \param handle a ProxyHandle pointer created by an environment
     */
    Proxy(ProxyHandle *handle);

    /*!
     * Is this Proxy have a handle?
     * \return true if the handle is set.
     */
    explicit operator bool(void) const;

    //! Get the handle held in this proxy object.
    std::shared_ptr<ProxyHandle> getHandle(void) const;

    /*!
     * Get the Environment that created this Object's Handle.
     */
    std::shared_ptr<ProxyEnvironment> getEnvironment(void) const;

    /*!
     * Convert this proxy to the specified ValueType.
     * \throws ProxyEnvironmentConvertError if conversion failed
     * \return the Proxy's value as ValueType
     */
    template <typename ValueType>
    ValueType convert(void) const;

    /*!
     * Templated conversion operator to assign Proxy to a target type.
     * \throws ProxyEnvironmentConvertError if object cannot be converted
     */
    template <typename ValueType>
    operator ValueType(void) const;

    //! Call a method with a return type and variable args
    template <typename ReturnType, typename... ArgsType>
    ReturnType call(const std::string &name, ArgsType&&... args) const;

    //! Call a method with a Proxy return and variable args
    template <typename... ArgsType>
    Proxy callProxy(const std::string &name, ArgsType&&... args) const;

    //! Call a method with a void return and variable args
    template <typename... ArgsType>
    void callVoid(const std::string &name, ArgsType&&... args) const;

    //! Call a field getter with specified return type
    template <typename ReturnType>
    ReturnType get(const std::string &name) const;

    //! Call a field getter with Proxy return type
    Proxy get(const std::string &name) const;

    //! Call a field setter
    template <typename ValueType>
    void set(const std::string &name, ValueType&& value) const;

    //! Call the function operator() with a Proxy return and variable args
    template <typename... ArgsType>
    Proxy operator()(ArgsType&&... args) const;

    /*!
     * Returns a negative integer, zero, or a positive integer as this object is
     * less than, equal to, or greater than the specified object.
     * \throws ProxyCompareError when the compare isnt possible
     * \param other the other proxy object to compare against
     * \return an int representing less than, equal to, or greater than
     */
    int compareTo(const Proxy &other) const;

    /*!
     * Get a hash code for the underlying object.
     * The hash code should be identical for equivalent objects.
     */
    size_t hashCode(void) const;

    /*!
     * Convert this proxy in this environment to a local object.
     * \throws ProxyEnvironmentConvertError if conversion failed
     * \return a new Object that contains something in local memory
     */
    Object toObject(void) const;

    /*!
     * Get the string representation of the Proxy.
     * The format of the string is highly specific,
     * depending upon the underlying object.
     */
    std::string toString(void) const;

    /*!
     * Get the class name of the underlying object.
     * The class name should be a unique identifier
     * for objects of the same type as the one contained.
     * This name is used to help convert proxies to local objects.
     */
    std::string getClassName(void) const;

    //! Comparable operator for stl containers
    bool operator<(const Proxy &obj) const;

    //! Comparable operator for stl containers
    bool operator>(const Proxy &obj) const;

private:
    std::shared_ptr<ProxyHandle> _handle;
};

/*!
 * The equals operators checks if two Proxies represent the same memory.
 * Use myProxy.compareTo(other) == 0 for an equality comparison.
 * \param lhs the left hand object of the comparison
 * \param rhs the right hand object of the comparison
 * \return true if the objects represent the same internal data
 */
POTHOS_API bool operator==(const Proxy &lhs, const Proxy &rhs);

} //namespace Pothos
