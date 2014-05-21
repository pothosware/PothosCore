//
// Remote/Node.hpp
//
// Remote node facilitates communication with a node on the network.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Remote/Client.hpp>
#include <string>
#include <vector>
#include <memory>
#include <ctime>

namespace Pothos {

/*!
 * Remote node facilitates communication with a node on the network.
 * The URI string should identify a server on the network.
 * The name and ID values are discovered by communicating with the node.
 * Also, the ID is a string that can uniquely identify a node.
 * The node's fields are stored in a persistent configuration,
 * and can be looked up while the nodes are potentially offline.
 */
class POTHOS_API RemoteNode
{
public:

    /*!
     * Create a Node with empty values.
     */
    RemoteNode(void);

    /*!
     * Create a node from a URI string.
     * This call does not attempt communication with the remote node.
     * Other fields will be filled out upon the first communication.
     *
     * \throws RemoteNodeError if the URI is malformed
     * \param uri a formatted string which specifies a server
     */
    RemoteNode(const std::string &uri);

    /*!
     * Create a node from a unique key.
     * The unique key will be looked up in a local configuration,
     * and the other fields filled in from this configuration.
     * This call does not incur any network communication.
     *
     * \throws RemoteNodeError when the key cant be found
     * \param key a unique key for a network Node connection
     * \return a new Node with all fields filled from config
     */
    static RemoteNode fromKey(const std::string &key);

    /*!
     * Try to perform a communication with the remote node.
     * \throws RemoteNodeError when the connection fails
     * \param timeoutUs the timeout to connect in microseconds
     */
    void communicate(const long timeoutUs = 100000);

    //! Get the name of this Node
    std::string getName(void) const;

    //! Get the unique ID of this Node
    std::string getId(void) const;

    //! Get the URI of this Node
    std::string getUri(void) const;

    /*!
     * Get the key of this Node.
     * A key is a hash generated from the Node fields.
     */
    std::string getKey(void) const;

    /*!
     * Get the time since the last access on this node.
     * \return time in seconds since the epoch (0 for never)
     */
    std::time_t getLastAccess(void) const;

    /*!
     * Is this Node online on the network?
     * \return the status since the last communication
     */
    bool isOnline(void) const;

    /*!
     * Make a remote client to run remote tasks on the node.
     * This call will spawn a new remote server and maintain
     * this server handle in a table for each value of "what".
     * The caller should use the "what" parameter to group
     * their proxy environments into different server processes.
     * Presumably, the user can make calls that may crash the process,
     * and therefore, this parameter can be used to create separate
     * environments for potentially crashing, and others that are safe.
     *
     * \throw RemoteNodeError if the node isnt online
     * \param what the name of a remote server handle
     * \param timeoutUs the timeout to connect in microseconds
     * \return a remote client to communicate with
     */
    RemoteClient makeClient(
        const std::string &what = "", const long timeoutUs = 100000);

    /*!
     * Create a temporary registration that lasts as long as the process.
     */
    void registerWithProcess(void) const;

    /*!
     * Add this node to the local registry.
     */
    void addToRegistry(void) const;

    /*!
     * Remove this node from the local registry.
     */
    void removeFromRegistry(void) const;

    /*!
     * Get a list of all nodes in the registry by key.
     * A node can be created with each ID using fromKey().
     * \return a list of node keys
     */
    static std::vector<std::string> listRegistryKeys(void);

private:
    void store(void) const;
    std::string _name;
    std::string _id;
    std::string _uri;
    std::string _key;
    bool _isOnline;
};

//! Are two remote nodes identical
POTHOS_API bool operator==(const RemoteNode &lhs, const RemoteNode &rhs);

} //namespace Pothos
