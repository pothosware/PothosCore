//
// Callable/CallInterfaceImpl.hpp
//
// Template implementation details for CallInterface.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/CallInterface.hpp>
#include <Pothos/Callable/Exception.hpp>
#include <Pothos/Object/ObjectImpl.hpp>
#include <utility> //std::forward

namespace Pothos {

/***********************************************************************
 * Templated call gateways with 0 args
 **********************************************************************/
template <typename ReturnType>
ReturnType CallInterface::call() const
{
    Object r = this->callObject();
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}


/***********************************************************************
 * Templated call gateways with 1 args
 **********************************************************************/
template <typename ReturnType, typename A0>
ReturnType CallInterface::call(A0 &&a0) const
{
    Object r = this->callObject(std::forward<A0>(a0));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0>
Object CallInterface::callObject(A0 &&a0) const
{
    Object args[1];
    args[0] = Object::make(std::forward<A0>(a0));
    return this->opaqueCall(args, 1);
}

template <typename A0>
void CallInterface::callVoid(A0 &&a0) const
{
    this->callObject(std::forward<A0>(a0));
}

/***********************************************************************
 * Templated call gateways with 2 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1>
ReturnType CallInterface::call(A0 &&a0, A1 &&a1) const
{
    Object r = this->callObject(std::forward<A0>(a0), std::forward<A1>(a1));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0, typename A1>
Object CallInterface::callObject(A0 &&a0, A1 &&a1) const
{
    Object args[2];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    return this->opaqueCall(args, 2);
}

template <typename A0, typename A1>
void CallInterface::callVoid(A0 &&a0, A1 &&a1) const
{
    this->callObject(std::forward<A0>(a0), std::forward<A1>(a1));
}

/***********************************************************************
 * Templated call gateways with 3 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2>
ReturnType CallInterface::call(A0 &&a0, A1 &&a1, A2 &&a2) const
{
    Object r = this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0, typename A1, typename A2>
Object CallInterface::callObject(A0 &&a0, A1 &&a1, A2 &&a2) const
{
    Object args[3];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    return this->opaqueCall(args, 3);
}

template <typename A0, typename A1, typename A2>
void CallInterface::callVoid(A0 &&a0, A1 &&a1, A2 &&a2) const
{
    this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2));
}

/***********************************************************************
 * Templated call gateways with 4 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3>
ReturnType CallInterface::call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const
{
    Object r = this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0, typename A1, typename A2, typename A3>
Object CallInterface::callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const
{
    Object args[4];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    return this->opaqueCall(args, 4);
}

template <typename A0, typename A1, typename A2, typename A3>
void CallInterface::callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3) const
{
    this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3));
}

/***********************************************************************
 * Templated call gateways with 5 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4>
ReturnType CallInterface::call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const
{
    Object r = this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0, typename A1, typename A2, typename A3, typename A4>
Object CallInterface::callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const
{
    Object args[5];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    args[4] = Object::make(std::forward<A4>(a4));
    return this->opaqueCall(args, 5);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4>
void CallInterface::callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4) const
{
    this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4));
}

/***********************************************************************
 * Templated call gateways with 6 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
ReturnType CallInterface::call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const
{
    Object r = this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
Object CallInterface::callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const
{
    Object args[6];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    args[4] = Object::make(std::forward<A4>(a4));
    args[5] = Object::make(std::forward<A5>(a5));
    return this->opaqueCall(args, 6);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
void CallInterface::callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5) const
{
    this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5));
}

/***********************************************************************
 * Templated call gateways with 7 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
ReturnType CallInterface::call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const
{
    Object r = this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Object CallInterface::callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const
{
    Object args[7];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    args[4] = Object::make(std::forward<A4>(a4));
    args[5] = Object::make(std::forward<A5>(a5));
    args[6] = Object::make(std::forward<A6>(a6));
    return this->opaqueCall(args, 7);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
void CallInterface::callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6) const
{
    this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6));
}

/***********************************************************************
 * Templated call gateways with 8 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
ReturnType CallInterface::call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const
{
    Object r = this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Object CallInterface::callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const
{
    Object args[8];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    args[4] = Object::make(std::forward<A4>(a4));
    args[5] = Object::make(std::forward<A5>(a5));
    args[6] = Object::make(std::forward<A6>(a6));
    args[7] = Object::make(std::forward<A7>(a7));
    return this->opaqueCall(args, 8);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
void CallInterface::callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7) const
{
    this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7));
}

/***********************************************************************
 * Templated call gateways with 9 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
ReturnType CallInterface::call(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const
{
    Object r = this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Object CallInterface::callObject(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const
{
    Object args[9];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    args[4] = Object::make(std::forward<A4>(a4));
    args[5] = Object::make(std::forward<A5>(a5));
    args[6] = Object::make(std::forward<A6>(a6));
    args[7] = Object::make(std::forward<A7>(a7));
    args[8] = Object::make(std::forward<A8>(a8));
    return this->opaqueCall(args, 9);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
void CallInterface::callVoid(A0 &&a0, A1 &&a1, A2 &&a2, A3 &&a3, A4 &&a4, A5 &&a5, A6 &&a6, A7 &&a7, A8 &&a8) const
{
    this->callObject(std::forward<A0>(a0), std::forward<A1>(a1), std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4), std::forward<A5>(a5), std::forward<A6>(a6), std::forward<A7>(a7), std::forward<A8>(a8));
}


} //namespace Pothos
