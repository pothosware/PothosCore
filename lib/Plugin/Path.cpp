// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin/Path.hpp>
#include <Pothos/Plugin/Exception.hpp>
#include <Poco/StringTokenizer.h>

static void validatePathString(const std::string &path)
{
    if (path == "/") return; //root path is ok, but may be funny for tokenizer since it ends in /
    bool first = true;
    for (const auto &name : Poco::StringTokenizer(path, "/"))
    {
        if (first and not name.empty())
        {
            throw Pothos::PluginPathError("Pothos::PluginPath("+path+")", "must start with root slash");
        }
        if (not first and name.empty())
        {
            throw Pothos::PluginPathError("Pothos::PluginPath("+path+")", "contains an empty name");
        }
        for (size_t i = 0; i < name.size(); i++)
        {
            if (name[i] >= 'A' and name[i] <= 'Z') continue;
            if (name[i] >= 'a' and name[i] <= 'z') continue;
            if (name[i] >= '0' and name[i] <= '9') continue;
            if (name[i] == '_' or  name[i] == '-') continue;
            throw Pothos::PluginPathError("Pothos::PluginPath("+path+")", "contains non-alphanumeric-slashy-dashy name");
        }
        first = false;
    }
}

Pothos::PluginPath::PluginPath(void):
    _path("/")
{
    validatePathString(_path);
}

Pothos::PluginPath::PluginPath(const std::string &path):
    _path(path)
{
    validatePathString(_path);
}

Pothos::PluginPath::PluginPath(const PluginPath &path0, const PluginPath &path1):
    _path(path0._path + path1._path) //should always be valid because of individual PluginPath rules
{
    validatePathString(_path);
}

Pothos::PluginPath::PluginPath(const char *path):
    _path(path)
{
    validatePathString(_path);
}

Pothos::PluginPath::PluginPath(const PluginPath &path):
    _path(path._path)
{
    return;
}

Pothos::PluginPath::~PluginPath(void)
{
    return;
}

Pothos::PluginPath &Pothos::PluginPath::operator=(const PluginPath &path)
{
    _path = path._path;
    return *this;
}

Pothos::PluginPath Pothos::PluginPath::join(const std::string &subPath) const
{
    const std::string joiner = (_path == "/")? "": "/"; //don't join with only root /
    return PluginPath(this->toString() + joiner + subPath);
}

std::vector<std::string> Pothos::PluginPath::listNodes(void) const
{
    std::vector<std::string> pathNames;
    Poco::StringTokenizer tok(_path, "/", Poco::StringTokenizer::TOK_IGNORE_EMPTY);
    for (Poco::StringTokenizer::Iterator it = tok.begin(); it != tok.end(); it++)
    {
        pathNames.push_back(*it);
    }
    return pathNames;
}

std::string Pothos::PluginPath::toString(void) const
{
    return _path;
}

bool Pothos::operator==(const PluginPath &lhs, const PluginPath &rhs)
{
    return lhs.toString() == rhs.toString();
}

#include <Pothos/Managed.hpp>

static auto managedPluginPath = Pothos::ManagedClass()
    .registerConstructor<Pothos::PluginPath>()
    .registerConstructor<Pothos::PluginPath, const std::string &>()
    .registerConstructor<Pothos::PluginPath, const Pothos::PluginPath &, const Pothos::PluginPath &>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PluginPath, join))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PluginPath, listNodes))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::PluginPath, toString))
    .commit("Pothos/PluginPath");
