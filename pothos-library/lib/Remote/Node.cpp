// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Remote.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/System/NodeInfo.hpp>
#include <Pothos/System/Paths.hpp>
#include <Poco/Util/PropertyFileConfiguration.h>
#include <Poco/Path.h>
#include <Poco/SingletonHolder.h>
#include <mutex>
#include <Poco/HashMap.h>
#include <Poco/Timestamp.h>
#include <Poco/URI.h>
#include <Poco/Hash.h>
#include <Poco/NumberFormatter.h>
#include <Poco/NumberParser.h>
#include <cassert>

/***********************************************************************
 * interface to configuration file
 **********************************************************************/
struct MyNodeConfig : Poco::Util::PropertyFileConfiguration
{
    static std::string getConfigPath(void)
    {
        Poco::Path path(Pothos::System::getUserConfigPath());
        path.append("Nodes.conf");
        return path.absolute().toString();
    }

    MyNodeConfig(void)
    {
        try
        {
            this->load(getConfigPath());
        }
        catch (...){}
        //TODO clean up unregistered entries that are super old
    }

    void store(void)
    {
        this->save(getConfigPath());
    }

    std::vector<std::string> listRootKeys(void)
    {
        std::vector<std::string> keys;
        this->enumerate("", keys);
        return keys;
    }

    std::vector<std::string> tempKeys;
};

static MyNodeConfig &getConfig(void)
{
    static Poco::SingletonHolder<MyNodeConfig> sh;
    return *sh.get();
}

static std::mutex &getMutex(void)
{
    static Poco::SingletonHolder<std::mutex> sh;
    return *sh.get();
}

/***********************************************************************
 * node implementation
 **********************************************************************/
Pothos::RemoteNode::RemoteNode(void):
    _isOnline(false)
{
    assert(not this->isOnline());
}

Pothos::RemoteNode::RemoteNode(const std::string &uriStr):
    _isOnline(false)
{
    assert(not this->isOnline());
    try
    {
        Poco::URI uri(uriStr);
        if (uri.getScheme() != "tcp")
        {
            throw RemoteNodeError("Pothos::RemoteNode("+uriStr+")", "unsupported URI scheme");
        }
        uri.normalize();
        _uri = uri.toString();
        _key = Poco::NumberFormatter::formatHex(Poco::hash(_uri));
        _name = uri.getHost(); //this is the name until we communicate
    }
    catch (const Poco::Exception &ex)
    {
        throw RemoteNodeError("Pothos::RemoteNode("+uriStr+")", ex.displayText());
    }

    //load anything we can from the configuration
    std::lock_guard<std::mutex> l(getMutex());
    const auto key = this->getKey();
    _id = getConfig().getString(key+".id", _id);
    _name = getConfig().getString(key+".name", _name);
    _uri = getConfig().getString(key+".uri", _uri);
}

Pothos::RemoteNode Pothos::RemoteNode::fromKey(const std::string &key)
{
    std::lock_guard<std::mutex> l(getMutex());
    RemoteNode n;
    n._id = getConfig().getString(key+".id", "");
    n._name = getConfig().getString(key+".name", "");
    n._uri = getConfig().getString(key+".uri", "");
    n._key = key;
    if (n.getKey().empty() or n.getUri().empty())
    {
        throw RemoteNodeError("Pothos::RemoteNode::fromKey("+key+")", "cannot find in config");
    }
    return n;
}

std::string Pothos::RemoteNode::getName(void) const
{
    return _name;
}

std::string Pothos::RemoteNode::getId(void) const
{
    return _id;
}

std::string Pothos::RemoteNode::getUri(void) const
{
    return _uri;
}

std::string Pothos::RemoteNode::getKey(void) const
{
    return _key;
}

void Pothos::RemoteNode::communicate(const long timeoutUs)
{
    try
    {
        this->makeClient("", timeoutUs);
    }
    catch (const RemoteNodeError &ex)
    {
        throw RemoteNodeError("Pothos::RemoteNode::communicate()", ex);
    }
}

bool Pothos::RemoteNode::isOnline(void) const
{
    return _isOnline;
}

std::time_t Pothos::RemoteNode::getLastAccess(void) const
{
    std::lock_guard<std::mutex> l(getMutex());
    return std::time_t(getConfig().getInt(this->getKey() + ".time", 0));
}

void Pothos::RemoteNode::store(void) const
{
    const auto key = this->getKey();
    if (not _id.empty()) getConfig().setString(key + ".id", _id);
    if (not _name.empty()) getConfig().setString(key + ".name", _name);
    if (not _uri.empty()) getConfig().setString(key + ".uri", _uri);
    getConfig().store();
}

/***********************************************************************
 * registry implementation
 **********************************************************************/
void Pothos::RemoteNode::registerWithProcess(void) const
{
    std::lock_guard<std::mutex> l(getMutex());
    getConfig().tempKeys.push_back(this->getKey());
    this->store();
}

void Pothos::RemoteNode::addToRegistry(void) const
{
    std::lock_guard<std::mutex> l(getMutex());
    getConfig().setBool(this->getKey() + ".registered", true);
    this->store();
}

void Pothos::RemoteNode::removeFromRegistry(void) const
{
    std::lock_guard<std::mutex> l(getMutex());
    getConfig().setBool(this->getKey() + ".registered", false);
    getConfig().store();
}

std::vector<std::string> Pothos::RemoteNode::listRegistryKeys(void)
{
    std::lock_guard<std::mutex> l(getMutex());
    std::vector<std::string> keys;
    for (const auto &key : getConfig().listRootKeys())
    {
        if (not key.empty() and
            getConfig().getBool(key + ".registered", false) and
            not getConfig().getString(key + ".uri", "").empty())
        {
            keys.push_back(key);
        }
    }
    keys.insert(keys.end(), getConfig().tempKeys.begin(), getConfig().tempKeys.end());
    return keys;
}

/***********************************************************************
 * get client implementation
 **********************************************************************/
typedef Poco::HashMap<std::string, Poco::HashMap<std::string, std::weak_ptr<Pothos::ProxyHandle>>> MyServerTable;

static MyServerTable &getServers(void)
{
    static Poco::SingletonHolder<MyServerTable> sh;
    return *sh.get();
}

Pothos::RemoteClient Pothos::RemoteNode::makeClient(const std::string &what, const long timeoutUs)
{
    // Find an already created server (proxy handle) in the table
    std::shared_ptr<Pothos::ProxyHandle> entry;
    {
        std::lock_guard<std::mutex> l(getMutex());
        entry = getServers()[this->getKey()][what].lock();
    }

    // Not created? make a new one
    if (not entry) try
    {
        //connect on a known port and make a remote environment to spawn a server
        RemoteClient client(this->getUri(), timeoutUs);
        auto env = client.makeEnvironment("managed");

        if (_id.empty())
        {
            auto nodeInfo = env->findProxy("Pothos/System/NodeInfo").call<Pothos::System::NodeInfo>("get");
            _id = nodeInfo.nodeId;
            _name = nodeInfo.nodeName;
            std::lock_guard<std::mutex> l(getMutex());
            this->store();
        }

        auto server = env->findProxy("Pothos/RemoteServer").callProxy("new", "tcp://0.0.0.0");

        //store the server into the table entry
        std::lock_guard<std::mutex> l(getMutex());
        entry = server.getHandle();
        getServers()[this->getKey()][what] = entry;
    }
    catch (const Exception &ex)
    {
        _isOnline = false;
        throw RemoteNodeError("Pothos::RemoteNode::makeClient()", ex);
    }

    //lookup the entry again if there was a creation race
    {
        std::lock_guard<std::mutex> l(getMutex());
        entry = getServers()[this->getKey()][what].lock();
    }

    //create a client to talk on the server in the table, hold a reference to this server
    Poco::URI clientUri(this->getUri());
    const std::string port = Proxy(entry).call<std::string>("getActualPort");
    clientUri.setPort(Poco::NumberParser::parse(port));
    Pothos::RemoteClient client(clientUri.toString());
    client.holdRef(Object(entry));

    //update access time after client connect above
    std::lock_guard<std::mutex> l(getMutex());
    getConfig().setInt(this->getKey() + ".time", int(Poco::Timestamp().epochTime()));
    getConfig().store();
    _isOnline = true;
    assert(this->isOnline());

    return client;
}

bool Pothos::operator==(const Pothos::RemoteNode &lhs, const Pothos::RemoteNode &rhs)
{
    return
        lhs.getName() == rhs.getName() and
        lhs.getId()   == rhs.getId()   and
        lhs.getUri()  == rhs.getUri();
}
