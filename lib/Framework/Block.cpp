// Copyright (c) 2014-2020 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include "Framework/ThreadEnvironment.hpp"
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Framework/InputPortImpl.hpp>
#include <Pothos/Framework/OutputPortImpl.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/String.h>

/***********************************************************************
 * threadpool calls
 **********************************************************************/
void Pothos::Block::setThreadPool(const ThreadPool &newThreadPool)
{
    if (_threadPool == newThreadPool) return; //no change

    //unregister if the old thread pool is valid
    if (_threadPool)
    {
        auto threads = std::static_pointer_cast<ThreadEnvironment>(_threadPool.getContainer());
        threads->unregisterTask(this);
    }

    //register if the new thread pool is valid
    if (newThreadPool)
    {
        auto threads = std::static_pointer_cast<ThreadEnvironment>(newThreadPool.getContainer());
        threads->registerTask(this,
            std::bind(&Pothos::WorkerActor::processTask, _actor.get(), std::placeholders::_1),
            std::bind(&Pothos::WorkerActor::wakeNoChange, _actor.get()));

        //configure the actor interface based on thread pool args
        //all we support for now is the default (wait) or spin mode
        _actor->enableWaitMode(threads->isWaitingEnabled());
    }

    //and save the reference to the new pool
    _threadPool = newThreadPool;
}

const Pothos::ThreadPool &Pothos::Block::getThreadPool(void) const
{
    return _threadPool;
}

/***********************************************************************
 * Block member implementation
 **********************************************************************/
Pothos::Block::Block(void):
    _actor(new WorkerActor(this))
{
    //set the default thread pool (registers)
    this->setThreadPool(ThreadPool(ThreadPoolArgs()));
}

Pothos::Block::~Block(void)
{
    //clear the thread pool (unregisters)
    this->setThreadPool(ThreadPool());
}

bool Pothos::Block::prepare(void)
{
    return true;
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

void Pothos::Block::propagateLabels(const InputPort *input)
{
    for (auto &entry : this->allOutputs())
    {
        auto &output = entry.second;
        if (output->isSignal()) continue;
        for (const auto &label : input->labels())
        {
            output->postLabel(label);
        }
    }
}

bool Pothos::Block::isActive(void) const
{
    return _actor->activeState;
}

Pothos::InputPort *Pothos::Block::setupInput(const std::string &name, const DType &dtype, const std::string &domain)
{
    if (name.empty()) throw PortAccessError("Pothos::Block::setupInput()", "empty name");
    if (_namedInputs.count(name) > 0) throw PortAccessError("Pothos::Block::setupInput("+name+")", "already registered");

    _actor->allocateInput(name, dtype, domain);
    return this->input(name);
}

Pothos::InputPort *Pothos::Block::setupInput(const size_t index, const DType &dtype, const std::string &domain)
{
    return this->setupInput(std::to_string(index), dtype, domain);
}

Pothos::OutputPort *Pothos::Block::setupOutput(const std::string &name, const DType &dtype, const std::string &domain)
{
    if (name.empty()) throw PortAccessError("Pothos::Block::setupOutput()", "empty name");
    if (_namedOutputs.count(name) > 0) throw PortAccessError("Pothos::Block::setupOutput("+name+")", "already registered");

    _actor->allocateOutput(name, dtype, domain);
    return this->output(name);
}

Pothos::OutputPort *Pothos::Block::setupOutput(const size_t index, const DType &dtype, const std::string &domain)
{
    return this->setupOutput(std::to_string(index), dtype, domain);
}

void Pothos::Block::registerCallable(const std::string &name, const Callable &call)
{
    _calls.insert(std::make_pair(name, call));

    //automatic registration of slots for calls that return void and are not "private"
    const bool isPrivate = name.front() == '_';
    const bool returnVoid = call.type(-1) == typeid(void);
    if (not isPrivate and returnVoid and _namedInputs.count(name) == 0)
    {
        this->registerSlot(name);
        _actor->automaticSlots.insert(name);
    }
}

void Pothos::Block::registerSignal(const std::string &name)
{
    if (name.empty()) throw PortAccessError("Pothos::Block::registerSignal()", "empty name");
    if (_namedOutputs.count(name) > 0) throw PortAccessError("Pothos::Block::registerSignal("+name+")", "already registered");

    _actor->allocateSignal(name);
}

void Pothos::Block::registerSlot(const std::string &name)
{
    if (name.empty()) throw PortAccessError("Pothos::Block::registerSlot()", "empty name");
    if (_actor->automaticSlots.count(name) != 0) return; //already registered automatically, no error
    if (_namedInputs.count(name) > 0) throw PortAccessError("Pothos::Block::registerSlot("+name+")", "already registered");

    _actor->allocateSlot(name);
}

void Pothos::Block::registerProbe(
    const std::string &name,
    const std::string &signalName_,
    const std::string &slotName_
)
{
    if (name.empty()) throw PortAccessError("Pothos::Block::registerProbe()", "empty name");
    auto signalName = signalName_;
    if (signalName.empty()) signalName = name + "Triggered";
    auto slotName = slotName_;
    if (slotName.empty()) slotName = "probe" + Poco::toUpper(name.substr(0, 1)) + name.substr(1);

    //create registration
    this->registerSlot(slotName);
    this->registerSignal(signalName);
    _probes[slotName] = std::make_pair(name, signalName);
}

Pothos::Object Pothos::Block::opaqueCallHandler(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs)
{
    //check if the name is a registered call
    const size_t numMatches = _calls.count(name);

    //no matches throw error
    if (numMatches == 0) throw Pothos::BlockCallNotFound("Pothos::Block::call("+name+")", "method does not exist in registry");

    //only one match, try the call and let it error out
    if (numMatches == 1) return _calls.find(name)->second.opaqueCall(inputArgs, numArgs);

    //otherwise try a match
    const auto ret = _calls.equal_range(name);
    for (auto it = ret.first; it != ret.second; ++it)
    {
        const auto &call = it->second;
        if (call.getNumArgs() != numArgs) continue;
        for (size_t i = 0; i < numArgs; i++)
        {
            if (not inputArgs[i].canConvert(call.type(i))) goto next;
        }
        return call.opaqueCall(inputArgs, numArgs);
        next: continue;
    }

    //could not find a match, error out
    throw Pothos::BlockCallNotFound("Pothos::Block::call("+name+")", "method match failed");
}

Pothos::Object Pothos::Block::opaqueCallMethod(const std::string &name, const Pothos::Object *inputArgs, const size_t numArgs) const
{
    //call into a signal
    auto out = _actor->outputs.find(name);
    if (out != _actor->outputs.end() and out->second->isSignal())
    {
        const ObjectVector args(inputArgs, inputArgs+numArgs);
        out->second->postMessage(Object(args));
        return Object();
    }

    //otherwise its a regular method
    auto it = _calls.find(name);
    if (it == _calls.end())
    {
        throw Pothos::BlockCallNotFound("Pothos::Block::call("+name+")", "method does not exist in registry");
    }

    ActorInterfaceLock lock(_actor.get());

    return const_cast<Block *>(this)->opaqueCallHandler(name, inputArgs, numArgs);
}

void Pothos::Block::yield(void)
{
    _actor->flagInternalChange();
}

std::shared_ptr<Pothos::BufferManager> Pothos::Block::getInputBufferManager(const std::string &, const std::string &)
{
    return Pothos::BufferManager::Sptr(); //abdicate
}

std::shared_ptr<Pothos::BufferManager> Pothos::Block::getOutputBufferManager(const std::string &, const std::string &)
{
    return Pothos::BufferManager::Sptr(); //abdicate
}

void Pothos::Block::setInputAlias(const std::string &portName, const std::string &alias)
{
    this->input(portName)->setAlias(alias);
}

void Pothos::Block::setOutputAlias(const std::string &portName, const std::string &alias)
{
    this->output(portName)->setAlias(alias);
}

std::vector<Pothos::PortInfo> Pothos::Block::inputPortInfo(void)
{
    std::vector<PortInfo> infos;
    for (const auto &name : _inputPortNames)
    {
        PortInfo info;
        info.name = name;
        info.alias = this->input(name)->alias();
        info.isSigSlot = this->input(name)->isSlot();
        info.dtype = this->input(name)->dtype();
        infos.push_back(info);
    }
    return infos;
}

std::vector<Pothos::PortInfo> Pothos::Block::outputPortInfo(void)
{
    std::vector<PortInfo> infos;
    for (const auto &name : _outputPortNames)
    {
        PortInfo info;
        info.name = name;
        info.alias = this->output(name)->alias();
        info.isSigSlot = this->output(name)->isSignal();
        info.dtype = this->output(name)->dtype();
        infos.push_back(info);
    }
    return infos;
}

#include <Pothos/Managed.hpp>

static Pothos::Block *getPointer(Pothos::Block &b)
{
    return &b;
}

static auto managedBlock = Pothos::ManagedClass()
    .registerClass<Pothos::Block>()
    .registerBaseClass<Pothos::Block, Pothos::Connectable>()
    .registerMethod("getPointer", &getPointer)
    .registerField(POTHOS_FCN_TUPLE(Pothos::Block, _actor))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, workInfo))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, isActive))

    //all of the setups with default args set
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, setThreadPool))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, getThreadPool))
    .registerMethod<Pothos::InputPort *, Pothos::Block, const std::string &, const Pothos::DType &, const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, setupInput))
    .registerMethod<Pothos::InputPort *, Pothos::Block, size_t, const Pothos::DType &, const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, setupInput))
    .registerMethod<Pothos::OutputPort *, Pothos::Block, const std::string &, const Pothos::DType &, const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, setupOutput))
    .registerMethod<Pothos::OutputPort *, Pothos::Block, size_t, const Pothos::DType &, const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, setupOutput))

    .registerMethod("setupInput", Pothos::Callable::make<Pothos::InputPort *, Pothos::Block, const std::string &, const Pothos::DType &, const std::string &>(&Pothos::Block::setupInput).bind("", 3))
    .registerMethod("setupInput", Pothos::Callable::make<Pothos::InputPort *, Pothos::Block, const std::string &, const Pothos::DType &, const std::string &>(&Pothos::Block::setupInput).bind("", 3).bind("byte", 2))
    .registerMethod("setupInput", Pothos::Callable::make<Pothos::InputPort *, Pothos::Block, size_t, const Pothos::DType &, const std::string &>(&Pothos::Block::setupInput).bind("", 3))
    .registerMethod("setupInput", Pothos::Callable::make<Pothos::InputPort *, Pothos::Block, size_t, const Pothos::DType &, const std::string &>(&Pothos::Block::setupInput).bind("", 3).bind("byte", 2))

    .registerMethod("setupOutput", Pothos::Callable::make<Pothos::OutputPort *, Pothos::Block, const std::string &, const Pothos::DType &, const std::string &>(&Pothos::Block::setupOutput).bind("", 3))
    .registerMethod("setupOutput", Pothos::Callable::make<Pothos::OutputPort *, Pothos::Block, const std::string &, const Pothos::DType &, const std::string &>(&Pothos::Block::setupOutput).bind("", 3).bind("byte", 2))
    .registerMethod("setupOutput", Pothos::Callable::make<Pothos::OutputPort *, Pothos::Block, size_t, const Pothos::DType &, const std::string &>(&Pothos::Block::setupOutput).bind("", 3))
    .registerMethod("setupOutput", Pothos::Callable::make<Pothos::OutputPort *, Pothos::Block, size_t, const Pothos::DType &, const std::string &>(&Pothos::Block::setupOutput).bind("", 3).bind("byte", 2))

    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, registerSignal))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, registerSlot))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, registerProbe))
    .registerMethod("registerProbe", Pothos::Callable(&Pothos::Block::registerProbe).bind("", 3))
    .registerMethod("registerProbe", Pothos::Callable(&Pothos::Block::registerProbe).bind("", 3).bind("", 2))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, inputs))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, allInputs))
    .registerMethod<Pothos::InputPort *, Pothos::Block, const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, input))
    .registerMethod<Pothos::InputPort *, Pothos::Block, size_t>(POTHOS_FCN_TUPLE(Pothos::Block, input))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, outputs))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, allOutputs))
    .registerMethod<Pothos::OutputPort *, Pothos::Block, const std::string &>(POTHOS_FCN_TUPLE(Pothos::Block, output))
    .registerMethod<Pothos::OutputPort *, Pothos::Block, size_t>(POTHOS_FCN_TUPLE(Pothos::Block, output))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::Block, yield))
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

/***********************************************************************
 * Register toString() outputs
 **********************************************************************/

#include <Pothos/Object/RegisterToString.hpp>

static std::string pothosBlockToString(const Pothos::Block& block)
{
    std::string ret = "Pothos::Block";
    if(!block.getName().empty())
    {
        ret += (" (name: " + block.getName() + ")");
    }

    return ret;
}

pothos_static_block(pothosRegisterBlockToString)
{
    Pothos::registerToStringFunc<Pothos::Block>(
        "Pothos/Block",
        &pothosBlockToString,
        true /*registerPointerTypes*/);
}
