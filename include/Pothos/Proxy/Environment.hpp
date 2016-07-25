///
/// \file Proxy/Environment.hpp
///
/// Definitions for the ProxyEnvironment interface class.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>
#include <Pothos/Proxy/Proxy.hpp>
#include <Pothos/Util/RefHolder.hpp>
#include <Pothos/Callable/Callable.hpp>
#include <utility> //std::forward
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <iosfwd>

namespace Pothos {

typedef std::map<std::string, std::string> ProxyEnvironmentArgs;

/*!
 * ProxyConvertPair typedef used in proxy -> local plugin registrations
 */
typedef std::pair<std::string, Pothos::Callable> ProxyConvertPair;

/*!
 * A ProxyEnvironment is the interaction point for dealing with managed objects.
 * Managed objects can take a variety of forms. For example:
 * - remote C++ objects on a server
 * - java objects in an embedded JVM
 * - python objects in an embedded interpreter
 * - or meta combinations of the above...
 */
class POTHOS_API ProxyEnvironment :
    public Util::RefHolder,
    public std::enable_shared_from_this<ProxyEnvironment>
{
public:
    typedef std::shared_ptr<ProxyEnvironment> Sptr;

    /*!
     * Create a new environment given the name of the factory.
     * Plugins for custom BufferManagers should be located in
     * the plugin registry: /proxy/environment/[name]
     * \throws ProxyEnvironmentFactoryError if the factory function fails.
     * \param name the name of a ProxyEnvironment factory in the plugin tree
     * \param args the proxy environment init arguments
     * \return a new shared pointer to a proxy environment
     */
    static Sptr make(const std::string &name, const ProxyEnvironmentArgs &args = ProxyEnvironmentArgs());

    //! Virtual destructor for subclassing
    virtual ~ProxyEnvironment(void);

    /*!
     * Get the unique ID of the node that this environment is running on.
     * Remote environments will report the node id of the remote server.
     */
    virtual std::string getNodeId(void) const;

    /*!
     * Get the unique ID of the process that this environment is running on.
     * Remote environments will report the process id of the remote server.
     */
    virtual std::string getUniquePid(void) const;

    /*!
     * Static method to get the unique ID of the caller process.
     * The ID is universally unique, based on the nodeId and pid.
     */
    static std::string getLocalUniquePid(void);

    /*!
     * Get the peering address of the connection for this environment.
     * Remote environment will report the peering address of the
     * caller as seen by the remote server's socket acceptor.
     */
    virtual std::string getPeeringAddress(void);

    /*!
     * Get the name of the environment.
     * This should be the same name passed into the factory.
     */
    virtual std::string getName(void) const = 0;

    /*!
     * Find a proxy object given its class name.
     * The resulting object will have calls to create
     * class instances, make static calls, and others.
     * \throws ProxyEnvironmentFindError if cannot find
     * \param name the name of a class in the registry
     * \return a Proxy representing the class
     */
    virtual Proxy findProxy(const std::string &name) = 0;

    /*!
     * Convert a local object into a proxy object in this environment.
     * \throws ProxyEnvironmentConvertError if conversion failed
     * \param local an Object that contains something in local memory
     * \return a new Proxy that represents the Object's contents
     */
    virtual Proxy convertObjectToProxy(const Object &local);

    /*!
     * Convert a local object into a proxy object in this environment.
     * Convenience templated version that takes the ValueType as input.
     */
    template <typename ValueType>
    Proxy makeProxy(ValueType &&local);

    /*!
     * Convert a proxy object in this environment into a local object.
     * \throws ProxyEnvironmentConvertError if conversion failed
     * \param proxy a Proxy that represents an object in the environment
     * \return a new Object that contains something in local memory
     */
    virtual Object convertProxyToObject(const Proxy &proxy);

    /*!
     * Serialize the contents of the proxy into a stream.
     * \throws ProxySerializeError is the operation cant complete
     * \param proxy the input proxy object to serialize
     * \param os the output stream for the serialized data
     */
    virtual void serialize(const Proxy &proxy, std::ostream &os) = 0;

    /*!
     * Deserialize the stream into the contents of a Proxy.
     * \throws ProxySerializeError is the operation cant complete
     * \param is the input stream holding serialized data
     * \return a new proxy from the serialized data
     */
    virtual Proxy deserialize(std::istream &is) = 0;
};

} //namespace Pothos

template <typename ValueType>
Pothos::Proxy Pothos::ProxyEnvironment::makeProxy(ValueType &&local)
{
    return convertObjectToProxy(Pothos::Object(std::forward<ValueType>(local)));
}
