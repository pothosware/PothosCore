///
/// \file Proxy/Handle.hpp
///
/// Definitions for the ProxyHandle interface class.
///
/// \copyright
/// Copyright (c) 2013-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy/Proxy.hpp>
#include <typeinfo>
#include <string>
#include <memory>

namespace Pothos {

class ProxyEnvironment;

/*!
 * The ProxyHandle is a pure interface class for creating ProxyObjects.
 * The templated calls from ProxyObject are convenience methods
 * that ultimately call into the ProxyHandle's call() method.
 * A ProxyHandle knows how to make proxy calls given arguments,
 * and then how to fetch and convert the return result from the call.
 */
class POTHOS_API ProxyHandle
{
public:
    /*!
     * Virtual destructor for subclassing.
     */
    virtual ~ProxyHandle(void);

    /*!
     * Get the Environment that created this Handle.
     */
    virtual std::shared_ptr<ProxyEnvironment> getEnvironment(void) const = 0;

    /*!
     * Make a call on this handle given method name and args.
     *
     * Argument note:
     * If one of the argument Proxy Objects is not in this environment,
     * it will be converted to a local Object and back into a ProxyObject
     * of the correct environment using the Environment convert calls.
     *
     * \throws ProxyHandleCallError if the call fails for some reason
     * \param name the name of the method
     * \param args an array of Proxy object arguments
     * \param numArgs the number of arguments in the array
     * \return a Proxy representing the result of the call
     */
    virtual Proxy call(const std::string &name, const Proxy *args, const size_t numArgs) = 0;

    /*!
     * Returns a negative integer, zero, or a positive integer as this object is
     * less than, equal to, or greater than the specified object.
     * \throws ProxyCompareError when the compare isnt possible
     * \param other the other proxy object to compare against
     * \return an int representing less than, equal to, or greater than
     */
    virtual int compareTo(const Proxy &other) const = 0;

    /*!
     * Get a hash code for the underlying object.
     * The hash code should be identical for equivalent objects.
     */
    virtual size_t hashCode(void) const = 0;

    /*!
     * Get the string representation of the Proxy.
     * The format of the string is highly specific,
     * depending upon the underlying object.
     */
    virtual std::string toString(void) const = 0;

    /*!
     * Get the class name of the underlying object.
     * The class name should be a unique identifier
     * for objects of the same type as the one contained.
     * This name is used to help convert proxies to local objects.
     */
    virtual std::string getClassName(void) const = 0;
};

} //namespace Pothos
