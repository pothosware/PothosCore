///
/// \file Framework/Topology.hpp
///
/// This file contains the interface for creating a topology of blocks.
///
/// \copyright
/// Copyright (c) 2014-2015 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

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

    /*!
     * Create a new empty topology in a shared ptr.
     * This is a convenience factory for Topology.
     */
    static std::shared_ptr<Topology> make(void);

    /*!
     * Create a topology from a JSON description.
     *
     * The "threadPools" field is an optional JSON object
     * where each entry contains thread pool arguments which are
     * documented by the ThreadPoolArgs JSON markup constructor.
     * A block can be associated to a particular thread pool
     * using the optional "threadPool" key and a pool name.
     * The special thread pool named "default" will apply
     * to all blocks that do not specify the "threadPool" key.
     *
     * Example JSON markup for a topology description:
     * \code {.json}
     * {
     *     "threadPools" : {
     *         "default" : {"priority" : 0.5},
     *         "myPool0" : {"yieldMode" : "SPIN"}
     *     },
     *     "blocks" : [
     *         {
     *             "id" : "id0",
     *             "path" : "/blocks/foo",
     *             "args" : [1, "hello"],
     *             "calls" : [
     *                 ["setFoo", true],
     *                 ["updateBaz", 3.14]
     *             ]
     *         },
     *         {
     *             "id" : "id1",
     *             "path" : "/blocks/bar",
     *             "threadPool" : "myPool0",
     *             "args" : [],
     *             "calls" : [
     *                 ["setBar", "OK"],
     *             ]
     *         }
     *     ],
     *     "connections", [
     *         ["self", "inputX", "id0", "in0"],
     *         ["id0", "out0", "id1", "in0"],
     *         ["id1", "out0", "self", "outputY"],
     *     ]
     * }
     * \endcode
     * \param json a JSON string or file path
     */
    static std::shared_ptr<Topology> make(const std::string &json);

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

    /*!
     * Get a vector of info about all of the input ports available.
     */
    std::vector<PortInfo> inputPortInfo(void);

    /*!
     * Get a vector of info about all of the output ports available.
     */
    std::vector<PortInfo> outputPortInfo(void);

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
     * Use a timeout value of 0.0 to wait forever for topology to become idle.
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
     * This call can be recursive and will disconnect all
     * on the other sub-topologies within this data flow.
     * No changes to the data flow occur until commit().
     * \param recursive true to recurse through sub-topologies
     */
    void disconnectAll(const bool recursive = false);

    //! Create a connection between a source port and a destination port.
    void _connect(
        const Object &src, const std::string &srcPort,
        const Object &dst, const std::string &dstPort);

    //! Remove a connection between a source port and a destination port.
    void _disconnect(
        const Object &src, const std::string &srcPort,
        const Object &dst, const std::string &dstPort);

    /*!
     * Export a function call on this topology to set/get parameters.
     * This call will automatically register a slot of the same name.
     * \param name the name of the callable
     * \param call the bound callable method
     */
    void registerCallable(const std::string &name, const Callable &call);

    /*!
     * Query performance statistics for all blocks in the topology.
     *
     * Example JSON markup for stats reporting:
     * (The actual stats markup has many more fields.)
     * \code {.json}
     * {
     *     "unique_id_of_blockA" : {
     *         "blockName" : "blockA",
     *         "numWorkCalls" : 12345,
     *         "outputStats" : [
     *              {"portName" : "0", totalElements : 42},
     *         ]
     *     },
     *     "unique_id_of_blockB" : {
     *         "blockName" : "blockB",
     *         "numWorkCalls" : 6789,
     *         "inputStats" : [
     *              {"portName" : "0", totalElements : 0},
     *              {"portName" : "1", totalElements : 100}
     *         ]
     *     }
     * }
     * \endcode
     *
     * \return a JSON formatted object string
     */
    std::string queryJSONStats(void);

    /*!
     * Dump the topology state to a JSON formatted string.
     * This call provides a structured view of the hierarchy.
     *
     * Example request object {"rendered" : true, "mode" : "flat"}
     *
     * Rendered options:
     *  - true: Include additional boundary crossing blocks.
     *  - false: Only blocks that are explicit to the design.
     *
     * Mode options:
     *  - "flat": Flattened topology no hierarchies.
     *  - "top": Top level blocks and hierarchies.
     *
     * Example JSON markup for presenting the topology:
     * \code {.json}
     * {
     *     "blocks" : {
     *         "uidblockA" : {
     *             "name" : "blockA",
     *             "outputs" : [
     *                 {"key": "outx", "type": "port"}
     *             ]
     *         },
     *         "uidblockB" : {
     *             "name" : "blockB",
     *             "inputs" : [
     *                 {"key": "in0", "type": "port"},
     *                 {"key": "setFoo", "type": "slot"}
     *             ],
     *             "outputs" : [
     *                 {"key": "out0", "type": "port"},
     *                 {"key": "barChanged", "type": "signal"}
     *             ]
     *         },
     *         "uidblockC" : {
     *             "name" : "blockC",
     *             "inputs" : [
     *                 {"key": "iny", "type": "port"}
     *             ],
     *             "blocks" : {#this is a hierarchy of blocks#},
     *             "connections" : {#this is a hierarchy of blocks#},
     *         }
     *     },
     *     "connections", [
     *         ["uidblockA", "outx", "uidblockB", "in0"],
     *         ["uidblockB", "out0", "uidblockC", "iny"]
     *     ]
     * }
     * \endcode
     *
     * \param request a JSON object string with key/value arguments
     * \return a JSON formatted object string
     */
    std::string dumpJSON(const std::string &request = "");

    /*!
     * Convert the topology to a string containing dot markup.
     * This markup can be passed into the dot tool to create a visual graph.
     * The markup can represent the connections as specified by the user,
     * or if flat is true, the complete flattened topology with
     * network blocks for crossing process/computer boundaries.
     *
     * Example request string {"mode" : "flat", "port" : "all"}
     *
     * Mode options:
     *  - "flat" Flattened topology no hierarchies.
     *  - "top" Top level blocks and hierarchies.
     *
     * Port options:
     *  - "all" Show all available IO ports.
     *  - "connected" Show connected ports only.
     *
     * \param request a JSON object string with configuration parameters
     * \return the dot markup as a string
     */
    std::string toDotMarkup(const std::string &request = "");

    /*!
     * Call a method on a derived instance with opaque input and return types.
     * \param name the name of the method as a string
     * \param inputArgs an array of input arguments
     * \param numArgs the size of the input array
     * \return the return value as type Object
     */
    Object opaqueCallMethod(const std::string &name, const Object *inputArgs, const size_t numArgs) const;

protected:
    /*!
     * The opaque call handler handles dispatching calls to registered methods.
     * The user may overload this call to install their own custom handler.
     * \throws BlockCallNotFound when no call registered for the provided name
     * \throws Exception when the registered call itself throws an exception
     * \param name the name of a call registered to this Block with registerCall()
     * \param inputArgs an array of input arguments wrapped in type Object
     * \param numArgs the number of arguments in the array inputArgs
     * \return the result of making the registered call, wrapped in type Object
     */
    virtual Object opaqueCallHandler(const std::string &name, const Object *inputArgs, const size_t numArgs);

private:
    Topology(const Topology &){} // non construction-copyable
    Topology &operator=(const Topology &){return *this;} // non copyable
public:
    struct Impl;
    std::shared_ptr<Impl> _impl;
};

} //namespace Pothos
