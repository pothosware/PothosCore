// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include "GraphObjects/GraphObject.hpp"
#include <QObject>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional> //std::hash
#include <unordered_set>

class GraphObject;
class GraphBlock;
class BlockEval;

namespace Pothos
{
    class Topology;
}

/*!
 * Information about a connection between src and dst ports.
 * This is everything important we need to know about connections,
 * but extracted so we can access it in a thread-safe manner.
 */
struct ConnectionInfo
{
    ConnectionInfo(void):
        srcBlock(nullptr),
        dstBlock(nullptr){}
    GraphBlock *srcBlock, *dstBlock;
    std::string srcPort, dstPort;
};

//! equality operator for ConnectionInfo for unordered_set
inline bool operator==(const ConnectionInfo &lhs, const ConnectionInfo &rhs)
{
    return
        (lhs.srcBlock == rhs.srcBlock) and
        (lhs.dstBlock == rhs.dstBlock) and
        (lhs.srcPort == rhs.srcPort) and
        (lhs.dstPort == rhs.dstPort);
}

//! hash support for ConnectionInfo for unordered_set
namespace std
{
    template<>
    struct hash<ConnectionInfo>
    {
        typedef ConnectionInfo argument_type;
        typedef std::size_t value_type;

        static value_type hash_combine(const value_type in0, const value_type in1)
        {
            return in0 ^ (in1 << 1);
        }

        value_type operator()(argument_type const& s) const
        {
            return hash_combine(
                hash_combine(std::hash<std::string>()(s.srcPort), std::hash<std::size_t>()(size_t(s.srcBlock))),
                hash_combine(std::hash<std::string>()(s.dstPort), std::hash<std::size_t>()(size_t(s.dstBlock))));
        }
    };
}

//! typedef for multiple connection informations
typedef std::unordered_set<ConnectionInfo> ConnectionInfos;

/*!
 * TopologyEval takes up to date connection information
 * and creates topology connections between block objects.
 */
class TopologyEval : public QObject
{
    Q_OBJECT
public:

    TopologyEval(void);

    ~TopologyEval(void);

    //! helper to parse graph objects into a list of thread-safe info
    static ConnectionInfos getConnectionInfo(const GraphObjectList &graphObjects);

    /*!
     * Pass-in up-to-date connection information.
     */
    void acceptConnectionInfo(const ConnectionInfos &);

    /*!
     * Pass-in up-to-date block eval objects.
     */
    void acceptBlockEvals(const std::map<GraphBlock *, std::shared_ptr<BlockEval>> &);

    /*!
     * Perform update work after changes applied.
     */
    void update(void);

private:
    ConnectionInfos _newConnectionInfo;
    ConnectionInfos _lastConnectionInfo;

    std::map<GraphBlock *, std::shared_ptr<BlockEval>> _newBlockEvals;
    std::map<GraphBlock *, std::shared_ptr<BlockEval>> _lastBlockEvals;

    //! The topology object thats executing this design
    Pothos::Topology *_topology;
};
