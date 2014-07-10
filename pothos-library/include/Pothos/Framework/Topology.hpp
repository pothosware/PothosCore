//
// Framework/Topology.hpp
//
// This file contains the interface for creating a topology of blocks.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/Connectable.hpp>
#include <Pothos/Framework/ThreadPool.hpp>
#include <Pothos/Object/Object.hpp>
#include <string>
#include <memory>
#include <iosfwd>

namespace Pothos {

/*!
 * The Topology class maintains a list of data flows.
 * The source of a flow is an output port on a Block or Topology.
 * The destination of a flow is an input port on a Block or Topology.
 * To create and destroy flows, make calls to connect and disconnect.
 * To create hierarchy, a topology's connections can be made with itself;
 * this action creates input and output ports for the topology.
 */
class POTHOS_API Topology : public Connectable
{
public:

    //! Create a new empty topology
    Topology(void);

    /*!
     * Cleanup and destroy a topology.
     * This call simply disconnects all data flows and commits the changes.
     */
    ~Topology(void);

    //! Set the thread pool used by all blocks in this topology.
    void setThreadPool(const ThreadPool &threadPool);

    //! Get the thread pool used by all blocks in this topology.
    const ThreadPool &getThreadPool(void) const;

    //! Get a list of input port names in order of connection
    std::vector<std::string> inputPortNames(void);

    //! Get a list of output port names in order of connection
    std::vector<std::string> outputPortNames(void);

    /*!
     * Commit changes made to the topology.
     * Actual data flows created by connect and disconnect
     * are not changed until a call to commit() is performed.
     * Once commit is called, actual data flow processing begins.
     * At this point the scheduler will call the block's work()
     * functions when the data at its inputs becomes available.
     */
    void commit(void);

    /*!
     * Wait for a period of data flow inactivity.
     * This call blocks until all flows become inactive for at least idleDuration seconds.
     * This call is intended primarily for unit testing purposes to allow the topology
     * to propagate test data through the entire flow from sources to sinks.
     * \param idleDuration the maximum number of seconds that flows may idle
     * \param timeout the maximum number of seconds to wait in this call
     * \return true if the flow graph became inactive before the timeout
     */
    bool waitInactive(const double idleDuration = 0.1, const double timeout = 1.0);

    /*!
     * Create a connection between a source port and a destination port.
     * \param src the data source (local/remote block/topology)
     * \param srcPort an identifier for the source port (string or index)
     * \param dst the data destination (local/remote block/topology)
     * \param dstPort an identifier for the destination port (string or index)
     */
    template <
        typename SrcType, typename SrcPortType,
        typename DstType, typename DstPortType>
    void connect(
        SrcType &&src, const SrcPortType &srcPort,
        DstType &&dst, const DstPortType &dstPort);

    /*!
     * Remove a connection between a source port and a destination port.
     * \param src the data source (local/remote block/topology)
     * \param srcPort an identifier for the source port (string or index)
     * \param dst the data destination (local/remote block/topology)
     * \param dstPort an identifier for the destination port (string or index)
     */
    template <
        typename SrcType, typename SrcPortType,
        typename DstType, typename DstPortType>
    void disconnect(
        SrcType &&src, const SrcPortType &srcPort,
        DstType &&dst, const DstPortType &dstPort);

    /*!
     * Disconnect all data flows inside this topology.
     * This call is recursive and will disconnect all
     * on the other sub-topologies in this data flow.
     * No changes to the data flow occur until commit().
     */
    void disconnectAll(void);

    //! Create a connection between a source port and a destination port.
    void _connect(
        const Object &src, const std::string &srcPort,
        const Object &dst, const std::string &dstPort);

    //! Remove a connection between a source port and a destination port.
    void _disconnect(
        const Object &src, const std::string &srcPort,
        const Object &dst, const std::string &dstPort);

    /*!
     * Convert the topology to a string containing dot markup.
     * This markup can be passed into the dot tool to create a visual graph.
     * The markup can represent the connections as specified by the user,
     * or if flat is true, the complete flattened topology with
     * network blocks for crossing process/computer boundaries.
     * \param flat true to show the flattened topology with network iogress
     * \return the dot markup as a string
     */
    std::string toDotMarkup(const bool flat = true);

public:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} //namespace Pothos
