//
// Framework/TopologyImpl.hpp
//
// Templated implementation details for the Topology class.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Framework/Topology.hpp>
#include <Pothos/Framework/ConnectableImpl.hpp>
#include <type_traits> //enable_if

namespace Pothos {

class Block; //forward declare for templates below
class Proxy; //forward declare for templates below

namespace Detail {

/***********************************************************************
 * templated conversions for connectable objects - blocks
 **********************************************************************/
template <typename T>
typename std::enable_if<std::is_base_of<Block, T>::value, Pothos::Object>::type
connObjToObject(T &value)
{
    return Pothos::Object(&static_cast<Block &>(value));
}

template <typename T>
typename std::enable_if<std::is_base_of<Block, T>::value, Pothos::Object>::type
connObjToObject(T *value)
{
    return Pothos::Object(static_cast<Block *>(value));
}

template <typename T>
typename std::enable_if<std::is_base_of<Block, T>::value, Pothos::Object>::type
connObjToObject(std::shared_ptr<T> value)
{
    return Pothos::Object(std::static_pointer_cast<Block>(value));
}

/***********************************************************************
 * templated conversions for connectable objects - topologies
 **********************************************************************/
template <typename T>
typename std::enable_if<std::is_base_of<Topology, T>::value, Pothos::Object>::type
connObjToObject(T &value)
{
    return Pothos::Object(&static_cast<Topology &>(value));
}


template <typename T>
typename std::enable_if<std::is_base_of<Topology, T>::value, Pothos::Object>::type
connObjToObject(T *value)
{
    return Pothos::Object(static_cast<Topology *>(value));
}

template <typename T>
typename std::enable_if<std::is_base_of<Topology, T>::value, Pothos::Object>::type
connObjToObject(std::shared_ptr<T> value)
{
    return Pothos::Object(std::static_pointer_cast<Topology>(value));
}

/***********************************************************************
 * templated conversions for connectable objects - proxies
 **********************************************************************/
template <typename T>
typename std::enable_if<std::is_same<Proxy, T>::value, Pothos::Object>::type
connObjToObject(const T &value)
{
    return Pothos::Object(value);
}

/***********************************************************************
 * templated conversions for port names
 **********************************************************************/
template <typename T>
typename std::enable_if<std::is_integral<T>::value, std::string>::type
portNameToStr(const T &value)
{
    return std::to_string(value);
}

template <typename T>
typename std::enable_if<!std::is_integral<T>::value, std::string>::type
portNameToStr(const T &value)
{
    return std::string(value);
}

} //namespace Detail
} //namespace Pothos

/***********************************************************************
 * templated implementation for connect and disconnect
 **********************************************************************/
template <
    typename SrcType, typename SrcPortType,
    typename DstType, typename DstPortType>
void Pothos::Topology::connect(
    SrcType &&src, const SrcPortType &srcPort,
    DstType &&dst, const DstPortType &dstPort)
{
    this->_connect(
        Detail::connObjToObject(src), Detail::portNameToStr(srcPort),
        Detail::connObjToObject(dst), Detail::portNameToStr(dstPort));
}

template <
    typename SrcType, typename SrcPortType,
    typename DstType, typename DstPortType>
void Pothos::Topology::disconnect(
    SrcType &&src, const SrcPortType &srcPort,
    DstType &&dst, const DstPortType &dstPort)
{
    this->_disconnect(
        Detail::connObjToObject(src), Detail::portNameToStr(srcPort),
        Detail::connObjToObject(dst), Detail::portNameToStr(dstPort));
}
