// Copyright (c) 2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/NumberParser.h>
#include <cassert>

/***********************************************************************
 * misc helper methods
 **********************************************************************/
int portNameToIndex(const std::string &name)
{
    unsigned value = -1;
    if (Poco::NumberParser::tryParseUnsigned(name, value)) return int(value);
    return -1;
}

/***********************************************************************
 * Port allocation templated implementation
 **********************************************************************/
template <typename ImplType, typename PortsType, typename PortNamesType>
void Pothos::WorkerActor::allocatePort(PortsType &ports, PortNamesType &portNames, const std::string &name, const DType &dtype)
{
    auto &port = ports[name];
    typedef typename PortsType::mapped_type::element_type T;
    port.reset(new T(new ImplType()));
    port->_impl->actor = this;
    port->_dtype = dtype;
    port->_name = name;
    port->_index = portNameToIndex(name);
    portNames.push_back(name);
    this->updatePorts();
}

template <typename ImplType, typename PortsType, typename IndexedPortsType, typename PortNamesType>
void Pothos::WorkerActor::autoAllocatePort(PortsType &ports, IndexedPortsType &indexedPorts, PortNamesType &portNames, const std::string &name)
{
    const int index = portNameToIndex(name);
    if (index == -1) return;
    if (ports.count(name) > 0) return;

    //indexed port does not exist, look for a lower index port and allocate
    for (int i = index-1; i >= 0; i--)
    {
        if (ports.count(std::to_string(i)) == 0) continue;
        this->allocatePort<ImplType>(ports, portNames, name, indexedPorts[i]->dtype());
        break;
    }

    //add to named ports list
    for (auto it = portNames.begin(); it != portNames.end(); it++)
    {
        if (portNameToIndex(*it)+1 == index)
        {
            portNames.insert(it+1, name);
            break;
        }
    }

    //TODO mark this port automatic so it can be deleted when unsubscribed

    this->updatePorts();
}

template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
void updatePortsT(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames)
{
    int numIndexed = 0;

    //refill named ports from ports
    namedPorts.clear();
    for (auto &entry : ports)
    {
        auto &port = *entry.second;
        assert(port.name() == entry.first);
        namedPorts[port.name()] = &port;
        if (port.index() != -1) numIndexed = std::max(numIndexed, port.index()+1);
    }

    //refill indexed ports from ports
    indexedPorts.resize(numIndexed, nullptr);
    for (size_t i = 0; i < indexedPorts.size(); i++)
    {
        auto it = ports.find(std::to_string(i));
        if (it == ports.end()) continue;
        indexedPorts[i] = it->second.get();
        assert(indexedPorts[i]->index() == i);
    }

    //autodelete from port names
    for (auto it = portNames.begin(); it != portNames.end();)
    {
        if (ports.count(*it) == 0)
        {
            portNames.erase(it);
            it = portNames.begin(); //iterator invalidated, restart loop
        }
        else it++;
    }
}

/***********************************************************************
 * Port allocation implementation
 **********************************************************************/
void Pothos::WorkerActor::allocateInput(const std::string &name, const DType &dtype)
{
    this->allocatePort<InputPortImpl>(block->_inputs, block->_inputPortNames, name, dtype);
}

void Pothos::WorkerActor::allocateOutput(const std::string &name, const DType &dtype)
{
    this->allocatePort<OutputPortImpl>(block->_outputs, block->_outputPortNames, name, dtype);
}

void Pothos::WorkerActor::allocateSignal(const std::string &name)
{
    this->allocateOutput(name, "byte");
    block->_outputs[name]->_impl->isSignal = true;
}

void Pothos::WorkerActor::allocateSlot(const std::string &name)
{
    this->allocateInput(name, "byte");
    block->_inputs[name]->_impl->isSlot = true;
}

void Pothos::WorkerActor::autoAllocateInput(const std::string &name)
{
    this->autoAllocatePort<InputPortImpl>(block->_inputs, block->_indexedInputs, block->_inputPortNames, name);
}

void Pothos::WorkerActor::autoAllocateOutput(const std::string &name)
{
    this->autoAllocatePort<OutputPortImpl>(block->_outputs, block->_indexedOutputs, block->_outputPortNames, name);
}

void Pothos::WorkerActor::updatePorts(void)
{
    block->_workInfo.inputPointers.resize(block->_indexedInputs.size());
    block->_workInfo.outputPointers.resize(block->_indexedOutputs.size());

    updatePortsT(block->_inputs, block->_namedInputs, block->_indexedInputs, block->_inputPortNames);
    updatePortsT(block->_outputs, block->_namedOutputs, block->_indexedOutputs, block->_outputPortNames);
}
