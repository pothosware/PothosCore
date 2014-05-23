// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/NumberParser.h>
#include <Poco/Exception.h> //SyntaxException
#include <cassert>

/***********************************************************************
 * misc helper methods
 **********************************************************************/
int portNameToIndex(const std::string &name)
{
    try
    {
        return Poco::NumberParser::parseUnsigned(name);
    }
    catch(const Poco::SyntaxException &){}
    return -1;
}

static void bufferManagerPushExternal(
    Pothos::BufferManager *mgr,
    std::shared_ptr<Theron::Framework> framework,
    const Theron::Address &addr,
    const Pothos::ManagedBuffer &buff
)
{
    BufferReturnMessage message;
    message.mgr = mgr;
    message.buff = buff;
    framework->Send(message, Theron::Address::Null(), addr);
}

/***********************************************************************
 * Port allocation implementation
 **********************************************************************/
template <typename ImplType, typename PortsType>
void Pothos::WorkerActor::__allocatePort(PortsType &ports, const std::string &name, const DType &dtype)
{
    auto &port = ports[name];
    typedef typename PortsType::mapped_type::element_type T;
    port.reset(new T(new ImplType()));
    port->_impl->actor = this;
    port->_dtype = dtype;
    port->_name = name;
    port->_index = portNameToIndex(name);
}

template <typename PortsType, typename NamedPortsType, typename IndexedPortsType>
void setupPorts(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts)
{
    int numIndexed = 0;

    namedPorts.clear();
    for (auto &entry : ports)
    {
        auto &port = *entry.second;
        assert(port.name() == entry.first);
        namedPorts[port.name()] = &port;
        if (port.index() != -1) numIndexed = std::max(numIndexed, port.index()+1);
    }

    indexedPorts.resize(numIndexed, nullptr);
    for (size_t i = 0; i < indexedPorts.size(); i++)
    {
        auto it = ports.find(std::to_string(i));
        if (it == ports.end()) continue;
        indexedPorts[i] = it->second.get();
        assert(indexedPorts[i]->index() == i);
    }
}

void Pothos::WorkerActor::allocateInput(const std::string &name, const DType &dtype)
{
    __allocatePort<InputPortImpl>(block->_inputs, name, dtype);
    setupPorts(block->_inputs, block->_namedInputs, block->_indexedInputs);
    block->_workInfo.inputPointers.resize(block->_indexedInputs.size());
    block->_inputPortNames.push_back(name);
}

void Pothos::WorkerActor::allocateOutput(const std::string &name, const DType &dtype)
{
    __allocatePort<OutputPortImpl>(block->_outputs, name, dtype);
    setupPorts(block->_outputs, block->_namedOutputs, block->_indexedOutputs);
    block->_workInfo.outputPointers.resize(block->_indexedOutputs.size());
    block->_outputPortNames.push_back(name);

    //setup the buffer return callback on the manager
    for (auto &entry : block->_outputs)
    {
        auto &port = *entry.second;
        auto &mgr = port._impl->bufferManager;
        mgr->setCallback(std::bind(&bufferManagerPushExternal,
            mgr.get(), block->_framework, this->GetAddress(), std::placeholders::_1));
    }
}
