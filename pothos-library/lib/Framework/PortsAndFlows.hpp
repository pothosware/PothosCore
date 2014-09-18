// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Proxy.hpp>
#include <functional> //std::hash

/***********************************************************************
 * A port contains a worker object and a port name
 **********************************************************************/
struct Port
{
    Pothos::Proxy obj; //!< object reference
    std::string name; //!< name of the port
    std::string uid; //!< sort on this key
    std::string toString(void) const
    {
        if (not obj) return "self["+name+"]";
        return obj.call<std::string>("getName")+"["+name+"]";
    }
};

inline bool operator==(const Port &lhs, const Port &rhs)
{
    assert(not lhs.uid.empty());
    assert(not rhs.uid.empty());
    if (not lhs.obj and not rhs.obj) return true; //both null
    if (not lhs.obj) return false;
    if (not rhs.obj) return false;
    if (lhs.uid != rhs.uid) return false;
    if (lhs.name != rhs.name) return false;
    return true;
}

namespace std
{
    template<>
    struct hash<Port>
    {
        typedef Port argument_type;
        typedef std::size_t value_type;

        value_type operator()(argument_type const& s) const
        {
            assert(not s.uid.empty());
            return std::hash<std::string>()(s.uid) ^
            (std::hash<std::string>()(s.name) << 1);
        }
    };
}

/***********************************************************************
 * A flow contains all the arguments for a connection
 **********************************************************************/
struct Flow
{
    Port src;
    Port dst;
    std::string toString(void) const
    {
        return src.toString()+"->"+dst.toString();
    }
};

inline bool operator==(const Flow &lhs, const Flow &rhs)
{
    if (not (rhs.src == lhs.src)) return false;
    if (not (rhs.dst == lhs.dst)) return false;
    return true;
}

namespace std
{
    template<>
    struct hash<Flow>
    {
        typedef Flow argument_type;
        typedef std::size_t value_type;

        value_type operator()(argument_type const& s) const
        {
            return std::hash<Port>()(s.src) ^
            (std::hash<Port>()(s.dst) << 1);
        }
    };
}
