// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Poco/SingletonHolder.h>

/***********************************************************************
 * Reusable threadpool for configuration purposes
 **********************************************************************/
struct ConfigThreadPool
{
    ConfigThreadPool(void)
    {
        framework.reset(new Theron::Framework());
    }
    std::shared_ptr<Theron::Framework> framework;
};

static ConfigThreadPool &getConfigThreadPool(void)
{
    static Poco::SingletonHolder<ConfigThreadPool> sh;
    return *sh.get();
}

/***********************************************************************
 * Block member implementation
 **********************************************************************/
Pothos::Block::Block(void):
    _framework(getConfigThreadPool().framework),
    _actor(new WorkerActor(this))
{
    return;
}

Pothos::Block::~Block(void)
{
    //Send a shutdown message and wait for response:
    //This allows the actor to finish with messages ahead of the shutdown message.
    Theron::Receiver receiver;
    ShutdownActorMessage message;
    _framework->Send(message, receiver.GetAddress(), _actor->GetAddress());
    receiver.Wait();
    _actor.reset();
}

void Pothos::Block::work(void)
{
    return;
}

void Pothos::Block::activate(void)
{
    return;
}

void Pothos::Block::deactivate(void)
{
    return;
}

void Pothos::Block::propagateLabels(const InputPort *input, const LabelIteratorRange &labels)
{
    const auto numConsumed = input->totalElements();
    for (auto &entry : this->allOutputs())
    {
        auto &output = entry.second;
        const auto numProduced = output->totalElements();
        for (const auto &label : labels)
        {
            assert(label.index <= numConsumed);
            auto newLabel = label;
            newLabel.index += numProduced;
            newLabel.index -= numConsumed;
            output->postLabel(newLabel);
        }
    }
}

void Pothos::Block::setupInput(const std::string &name, const DType &dtype)
{
    _actor->allocateInput(name, dtype);
}

void Pothos::Block::setupInput(const size_t index, const DType &dtype)
{
    _actor->allocateInput(std::to_string(index), dtype);
}

void Pothos::Block::setupOutput(const std::string &name, const DType &dtype)
{
    _actor->allocateOutput(name, dtype);
}

void Pothos::Block::setupOutput(const size_t index, const DType &dtype)
{
    _actor->allocateOutput(std::to_string(index), dtype);
}

void Pothos::Block::registerCallable(const std::string &name, const Callable &call)
{
    _calls[name] = call;
}

Pothos::Object Pothos::Block::opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
{
    auto it = _calls.find(name);
    if (it == _calls.end())
    {
        throw Pothos::BlockCallNotFound("Pothos::Block::call("+name+")", "method does not exist in registry");
    }
    return it->second.opaqueCall(inputArgs, numArgs);
}

Pothos::Object Pothos::Block::opaqueCall(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
{
    OpaqueCallMessage message;
    message.name = name;
    message.inputArgs = inputArgs;
    message.numArgs = numArgs;

    InfoReceiver<OpaqueCallResultMessage> receiver;
    _actor->GetFramework().Send(message, receiver.GetAddress(), _actor->GetAddress());

    OpaqueCallResultMessage result = receiver.WaitInfo();
    if (result.error) result.error->rethrow();
    return result.obj;
}

#include <Pothos/Managed.hpp>

static std::shared_ptr<Pothos::WorkerActor> getActor(const Pothos::Block &block)
{
    return block._actor;
}

static const std::string &getUid(const Pothos::Block &b)
{
    return b.uid();
}

static auto managedBlock = Pothos::ManagedClass()
    .registerClass<Pothos::Block>()
    .registerWildcardMethod(&Pothos::Block::opaqueCall)
    .registerMethod("uid", &getUid)
    .registerMethod("actor", &getActor)
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, workInfo))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, inputPortNames))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, outputPortNames))
    .registerMethod<const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, setupInput))
    .registerMethod<const size_t>(POTHOS_FCN_TUPLE(Pothos::Block, setupInput))
    .registerMethod<const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, setupOutput))
    .registerMethod<const size_t>(POTHOS_FCN_TUPLE(Pothos::Block, setupOutput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, inputs))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, allInputs))
    .registerMethod<const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, input))
    .registerMethod<const size_t>(POTHOS_FCN_TUPLE(Pothos::Block, input))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, outputs))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, allOutputs))
    .registerMethod<const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, output))
    .registerMethod<const size_t>(POTHOS_FCN_TUPLE(Pothos::Block, output))
    .commit("Pothos/Block");

template <typename PortType>
size_t portVectorSize(const std::vector<PortType*> &ports)
{
    return ports.size();
}

template <typename PortType>
PortType *portVectorAt(const std::vector<PortType*> &ports, const size_t index)
{
    return ports.at(index);
}

template <typename PortType>
size_t portMapSize(const std::map<std::string, PortType*> &ports)
{
    return ports.size();
}

template <typename PortType>
PortType *portMapAt(const std::map<std::string, PortType*> &ports, const std::string &key)
{
    return ports.at(key);
}

template <typename PortType>
std::vector<std::string> portMapKeys(const std::map<std::string, PortType*> &ports)
{
    std::vector<std::string> keys;
    for (const auto &pair : ports) keys.push_back(pair.first);
    return keys;
}

static auto managedInputPortVector = Pothos::ManagedClass()
    .registerClass<std::vector<Pothos::InputPort*>>()
    .registerMethod("size", &portVectorSize<Pothos::InputPort>)
    .registerMethod("at", &portVectorAt<Pothos::InputPort>)
    .commit("Pothos/InputPortVector");

static auto managedInputPortMap = Pothos::ManagedClass()
    .registerClass<std::map<std::string, Pothos::InputPort*>>()
    .registerMethod("size", &portMapSize<Pothos::InputPort>)
    .registerMethod("at", &portMapAt<Pothos::InputPort>)
    .registerMethod("keys", &portMapKeys<Pothos::InputPort>)
    .commit("Pothos/InputPortMap");

static auto managedOutputPortVector = Pothos::ManagedClass()
    .registerClass<std::vector<Pothos::OutputPort*>>()
    .registerMethod("size", &portVectorSize<Pothos::OutputPort>)
    .registerMethod("at", &portVectorAt<Pothos::OutputPort>)
    .commit("Pothos/OutputPortVector");

static auto managedOutputPortMap = Pothos::ManagedClass()
    .registerClass<std::map<std::string, Pothos::OutputPort*>>()
    .registerMethod("size", &portMapSize<Pothos::OutputPort>)
    .registerMethod("at", &portMapAt<Pothos::OutputPort>)
    .registerMethod("keys", &portMapKeys<Pothos::OutputPort>)
    .commit("Pothos/OutputPortMap");
