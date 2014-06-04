//
// Proxy/Proxy.hpp
//
// Definitions for the Proxy wrapper class.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
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
    pothos_explicit operator bool(void) const;

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

    //! Call a method with a return type and 0 args
    template <typename ReturnType>
    ReturnType call(const std::string &name) const;

    //! Call a method with a Proxy return and 0 args
    inline
    Proxy callProxy(const std::string &name) const;

    //! Call a method with a void return and 0 args
    inline
    void call(const std::string &name) const;
    //! Call a method with a return type and 1 args
    template <typename ReturnType, typename A0>
    ReturnType call(const std::string &name, const A0 &a0) const;

    //! Call a method with a Proxy return and 1 args
    template <typename A0>
    Proxy callProxy(const std::string &name, const A0 &a0) const;

    //! Call a method with a void return and 1 args
    template <typename A0>
    void call(const std::string &name, const A0 &a0) const;
    //! Call a method with a return type and 2 args
    template <typename ReturnType, typename A0, typename A1>
    ReturnType call(const std::string &name, const A0 &a0, const A1 &a1) const;

    //! Call a method with a Proxy return and 2 args
    template <typename A0, typename A1>
    Proxy callProxy(const std::string &name, const A0 &a0, const A1 &a1) const;

    //! Call a method with a void return and 2 args
    template <typename A0, typename A1>
    void call(const std::string &name, const A0 &a0, const A1 &a1) const;
    //! Call a method with a return type and 3 args
    template <typename ReturnType, typename A0, typename A1, typename A2>
    ReturnType call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2) const;

    //! Call a method with a Proxy return and 3 args
    template <typename A0, typename A1, typename A2>
    Proxy callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2) const;

    //! Call a method with a void return and 3 args
    template <typename A0, typename A1, typename A2>
    void call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2) const;
    //! Call a method with a return type and 4 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3>
    ReturnType call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3) const;

    //! Call a method with a Proxy return and 4 args
    template <typename A0, typename A1, typename A2, typename A3>
    Proxy callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3) const;

    //! Call a method with a void return and 4 args
    template <typename A0, typename A1, typename A2, typename A3>
    void call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3) const;
    //! Call a method with a return type and 5 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4>
    ReturnType call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4) const;

    //! Call a method with a Proxy return and 5 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    Proxy callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4) const;

    //! Call a method with a void return and 5 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    void call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4) const;
    //! Call a method with a return type and 6 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    ReturnType call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5) const;

    //! Call a method with a Proxy return and 6 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    Proxy callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5) const;

    //! Call a method with a void return and 6 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    void call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5) const;
    //! Call a method with a return type and 7 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    ReturnType call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6) const;

    //! Call a method with a Proxy return and 7 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    Proxy callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6) const;

    //! Call a method with a void return and 7 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    void call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6) const;
    //! Call a method with a return type and 8 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    ReturnType call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7) const;

    //! Call a method with a Proxy return and 8 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    Proxy callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7) const;

    //! Call a method with a void return and 8 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    void call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7) const;
    //! Call a method with a return type and 9 args
    template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    ReturnType call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8) const;

    //! Call a method with a Proxy return and 9 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    Proxy callProxy(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8) const;

    //! Call a method with a void return and 9 args
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    void call(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8) const;

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

inline Pothos::Proxy Pothos::Proxy::callProxy(const std::string &name) const
{
    return this->call<Proxy>(name);
}

inline void Pothos::Proxy::call(const std::string &name) const
{
    this->call<Proxy>(name);
}
