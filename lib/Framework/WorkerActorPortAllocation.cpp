// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <cassert>
#include <algorithm> //min/max
#include <cctype> //isdigit

/***********************************************************************
 * misc helper methods
 **********************************************************************/
static int portNameToIndex(const std::string &name)
{
    //check if all characters are digits or return -1
    for (const auto &ch : name)
    {
        if (not std::isdigit(ch)) return -1;
    }

    //convert the string to an integer
    return std::stoi(name);
}

/***********************************************************************
 * Port allocation templated implementation
 **********************************************************************/
template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
void Pothos::WorkerActor::allocatePort(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames,
    const std::string &name, const DType &dtype, const std::string &domain, const bool automatic)
{
    auto &port = ports[name];
    typedef typename PortsType::mapped_type::element_type T;
    port.reset(new T());
    port->_actor = this;
    port->_dtype = dtype;
    port->_domain = domain;
    port->_name = name;
    port->_index = portNameToIndex(name);

    //record port name in order of allocation
    portNames.push_back(name);

    //install port pointer into named ports
    namedPorts[name] = port.get();

    //install pointer into indexed ports (if its indexable)
    if (port->index() != -1)
    {
        const auto index = size_t(port->index());
        indexedPorts.resize(std::max(index+1, indexedPorts.size()), nullptr);
        indexedPorts[index] = port.get();
    }

    //record automatically created ports
    if (automatic) this->automaticPorts.insert(port.get());

    //resizes work info indexable pointers
    this->updatePorts();
}

template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
void Pothos::WorkerActor::autoAllocatePort(PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames, const std::string &name)
{
    const int index = portNameToIndex(name);
    if (index == -1) return;
    if (ports.count(name) > 0) return;

    //indexed port does not exist, look for a lower index port and allocate
    for (int i = index-1; i >= 0; i--)
    {
        if (ports.count(std::to_string(i)) == 0) continue;
        this->allocatePort(ports, namedPorts, indexedPorts, portNames, name, indexedPorts[i]->dtype(), indexedPorts[i]->domain(), true/*auto*/);
        break;
    }
}

/***********************************************************************
 * Port allocation implementation
 **********************************************************************/
void Pothos::WorkerActor::allocateInput(const std::string &name, const DType &dtype, const std::string &domain)
{
    this->allocatePort(this->inputs, block->_namedInputs, block->_indexedInputs, block->_inputPortNames, name, dtype, domain);
}

void Pothos::WorkerActor::allocateOutput(const std::string &name, const DType &dtype, const std::string &domain)
{
    this->allocatePort(this->outputs, block->_namedOutputs, block->_indexedOutputs, block->_outputPortNames, name, dtype, domain);
}

void Pothos::WorkerActor::allocateSignal(const std::string &name)
{
    this->allocateOutput(name, "", "");
    this->outputs[name]->_isSignal = true;
}

void Pothos::WorkerActor::allocateSlot(const std::string &name)
{
    this->allocateInput(name, "", "");
    this->inputs[name]->_isSlot = true;
}

void Pothos::WorkerActor::autoAllocateInput(const std::string &name)
{
    this->autoAllocatePort(this->inputs, block->_namedInputs, block->_indexedInputs, block->_inputPortNames, name);
}

void Pothos::WorkerActor::autoAllocateOutput(const std::string &name)
{
    this->autoAllocatePort(this->outputs, block->_namedOutputs, block->_indexedOutputs, block->_outputPortNames, name);
}

void Pothos::WorkerActor::updatePorts(void)
{
    //resize the work info pointer arrays
    block->_workInfo.inputPointers.resize(block->_indexedInputs.size());
    block->_workInfo.outputPointers.resize(block->_indexedOutputs.size());
}

/***********************************************************************
 * Port deletion implementation
 **********************************************************************/
template <typename PortsType, typename NamedPortsType, typename IndexedPortsType, typename PortNamesType>
void Pothos::WorkerActor::autoDeletePort(const std::string &name, PortsType &ports, NamedPortsType &namedPorts, IndexedPortsType &indexedPorts, PortNamesType &portNames)
{
    auto it = ports.find(name);
    if (it == ports.end()) return;

    auto &port = *it->second;
    if (not port._subscribers.empty()) return;

    //remove from the auto ports list
    {
        auto idx = this->automaticPorts.find(&port);
        if (idx == this->automaticPorts.end()) return;
        this->automaticPorts.erase(idx);
    }

    //remove it from indexed ports and strip null ports
    if (port.index() >= 0 and size_t(port.index()) < indexedPorts.size())
    {
        indexedPorts[port.index()] = nullptr;
    }
    while (not indexedPorts.empty() and indexedPorts.back() == nullptr)
    {
        indexedPorts.resize(int(indexedPorts.size())-1);
    }

    //named port removal
    if (namedPorts.count(port.name()) != 0)
    {
        namedPorts.erase(port.name());
    }

    //port names removal
    {
        auto idx = std::find(portNames.begin(), portNames.end(), port.name());
        if (idx != portNames.end()) portNames.erase(idx);
    }

    //remove from ports itself
    ports.erase(it);
}

void Pothos::WorkerActor::autoDeleteInput(const std::string &name)
{
    this->autoDeletePort(name, this->inputs, block->_namedInputs, block->_indexedInputs, block->_inputPortNames);
}

void Pothos::WorkerActor::autoDeleteOutput(const std::string &name)
{
    this->autoDeletePort(name, this->outputs, block->_namedOutputs, block->_indexedOutputs, block->_outputPortNames);
}
