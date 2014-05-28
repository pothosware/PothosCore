//
// Framework/SignalEmitterImpl.hpp
//
// Template method implementations for SignalEmitter.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/SignalEmitter.hpp>

namespace Pothos {

template <typename A0>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0)
{
    Object args[1];
    args[0] = Object::make(std::forward<A0>(a0));
    this->emitSignalArgs(name, args, 1);
}

template <typename A0, typename A1>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0, const A1 &a1)
{
    Object args[2];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    this->emitSignalArgs(name, args, 2);
}

template <typename A0, typename A1, typename A2>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2)
{
    Object args[3];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    this->emitSignalArgs(name, args, 3);
}

template <typename A0, typename A1, typename A2, typename A3>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3)
{
    Object args[4];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    this->emitSignalArgs(name, args, 4);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    Object args[5];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    args[4] = Object::make(std::forward<A4>(a4));
    this->emitSignalArgs(name, args, 5);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    Object args[6];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    args[4] = Object::make(std::forward<A4>(a4));
    args[5] = Object::make(std::forward<A5>(a5));
    this->emitSignalArgs(name, args, 6);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    Object args[7];
    args[0] = Object::make(std::forward<A0>(a0));
    args[1] = Object::make(std::forward<A1>(a1));
    args[2] = Object::make(std::forward<A2>(a2));
    args[3] = Object::make(std::forward<A3>(a3));
    args[4] = Object::make(std::forward<A4>(a4));
    args[5] = Object::make(std::forward<A5>(a5));
    args[6] = Object::make(std::forward<A6>(a6));
    this->emitSignalArgs(name, args, 7);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
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
    this->emitSignalArgs(name, args, 8);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
void SignalEmitter::emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
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
    this->emitSignalArgs(name, args, 9);
}


} //namespace Pothos
