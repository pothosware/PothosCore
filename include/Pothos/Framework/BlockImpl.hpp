///
/// \file Framework/BlockImpl.hpp
///
/// This file contains inline definitions for Block members.
///
/// \copyright
/// Copyright (c) 2014-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/ConnectableImpl.hpp>
#include <Pothos/Framework/Exception.hpp>
#include <Pothos/Object/Containers.hpp>
#include <utility> //std::forward

inline const Pothos::WorkInfo &Pothos::Block::workInfo(void) const
{
    return _workInfo;
}

inline Pothos::InputPort *Pothos::Block::input(const std::string &name) const
{
    auto it = _namedInputs.find(name);
    if (it == _namedInputs.end()) throw PortAccessError(
        "Pothos::Block::input("+name+")", "input port name does not exist");
    return it->second;
}

inline Pothos::InputPort *Pothos::Block::input(const size_t index) const
{
    if (index >= _indexedInputs.size()) throw PortAccessError(
        "Pothos::Block::input("+std::to_string(index)+")", "input port index does not exist");
    return _indexedInputs[index];
}

inline Pothos::OutputPort *Pothos::Block::output(const std::string &name) const
{
    auto it = _namedOutputs.find(name);
    if (it == _namedOutputs.end()) throw PortAccessError(
        "Pothos::Block::output("+name+")", "output port name does not exist");
    return it->second;
}

inline Pothos::OutputPort *Pothos::Block::output(const size_t index) const
{
    if (index >= _indexedOutputs.size()) throw PortAccessError(
        "Pothos::Block::output("+std::to_string(index)+")", "output port index does not exist");
    return _indexedOutputs[index];
}

inline const std::vector<Pothos::InputPort*> &Pothos::Block::inputs(void) const
{
    return _indexedInputs;
}

inline const std::vector<Pothos::OutputPort*> &Pothos::Block::outputs(void) const
{
    return _indexedOutputs;
}

inline const std::map<std::string, Pothos::InputPort*> &Pothos::Block::allInputs(void) const
{
    return _namedInputs;
}

inline const std::map<std::string, Pothos::OutputPort*> &Pothos::Block::allOutputs(void) const
{
    return _namedOutputs;
}

template <typename... ArgsType>
void Pothos::Block::emitSignal(const std::string &name, ArgsType&&... args)
{
    const auto it = _namedOutputs.find(name);
    if (it == _namedOutputs.end() or not it->second->isSignal()) throw PortAccessError(
        "Pothos::Block::emitSignal("+name+")", "signal port does not exist");

    const ObjectVector objArgs{{Object(std::forward<ArgsType>(args))...}};
    it->second->postMessage(std::move(objArgs));
}
