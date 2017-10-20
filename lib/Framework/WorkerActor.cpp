// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "Framework/WorkerActor.hpp"
#include <Pothos/Framework/InputPortImpl.hpp>
#include <Pothos/Framework/OutputPortImpl.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Poco/Format.h>
#include <Poco/Logger.h>
#include <cassert>
#include <algorithm> //min/max
#include <json.hpp>

using json = nlohmann::json;

//! Helper routine to deal with automatically accumulating time durations
struct TimeAccumulator
{
    inline TimeAccumulator(std::chrono::high_resolution_clock::duration &t):
        t(t), start(std::chrono::high_resolution_clock::now())
    {
        return;
    }
    inline ~TimeAccumulator(void)
    {
        t += std::chrono::high_resolution_clock::now() - start;
    }
    std::chrono::high_resolution_clock::duration &t;
    const std::chrono::high_resolution_clock::time_point start;
};

/***********************************************************************
 * buffer manager helpers
 **********************************************************************/
std::string Pothos::WorkerActor::getBufferMode(const std::string &name, const std::string &domain, const bool isInput)
{
    ActorInterfaceLock lock(this);

    //signals and slots always abdicate, they don't care about buffers
    if (isInput and block->input(name)->isSlot()) return "ABDICATE";
    if (not isInput and block->output(name)->isSignal()) return "ABDICATE";

    //look up the mode in the cache from a previous request
    auto &mode = bufferModeCache[isInput][name][domain];
    if (not mode.empty()) return mode;

    //query the mode from the block's buffer manager overloads
    //the result is cached for subsequent calls to getBufferManager()
    //only the last manager for each port is actually stored (in tmp)
    try
    {
        auto &m = bufferManagerTmpCache[isInput][name]; m.reset();
        m = isInput? block->getInputBufferManager(name, domain) : block->getOutputBufferManager(name, domain);
        mode = m? "CUSTOM" : "ABDICATE";
        bufferManagerCache[isInput][name][domain] = m;
    }
    catch (const PortDomainError &)
    {
        mode = "ERROR";
    }
    return mode;
}

Pothos::BufferManager::Sptr Pothos::WorkerActor::getBufferManager(const std::string &name, const std::string &domain, const bool isInput)
{
    ActorInterfaceLock lock(this);
    return this->getBufferManagerNoLock(name, domain, isInput);
}

Pothos::BufferManager::Sptr Pothos::WorkerActor::getBufferManagerNoLock(const std::string &name, const std::string &domain, const bool isInput)
{
    //check the cache for a manager thats still in use
    auto &weakMgr = bufferManagerCache[isInput][name][domain];
    auto m = weakMgr.lock();

    //try to get the manager and make one if its null
    if (not m) m = isInput? block->getInputBufferManager(name, domain) : block->getOutputBufferManager(name, domain);
    if (not m) m = BufferManager::make("generic", BufferManagerArgs());
    else if (not m->isInitialized()) m->init(BufferManagerArgs()); //TODO pass this in from somewhere

    //store the new buffer manager to the cache
    weakMgr = m;
    return m;
}

void Pothos::WorkerActor::setOutputBufferManager(const std::string &name, const BufferManager::Sptr &manager)
{
    ActorInterfaceLock lock(this);
    outputs.at(name)->bufferManagerSetup(manager);
}

void Pothos::WorkerActor::ensureOutputBufferManagerNoLock(const std::string &name)
{
    auto &port = *this->outputs.at(name);

    //signal ports ignored, they don't need managers
    if (port.isSignal()) return;

    //without subscribers, provide the manager from a local source
    if (port._subscribers.empty())
    {
        BufferManager::Sptr mgr;
        try {mgr = this->getBufferManagerNoLock(name, "", false);}
        catch (...){}
        port.bufferManagerSetup(mgr);
    }

    //ensure that a buffer manager has been installed
    if (not port._bufferManager) throw RuntimeException(
        "Pothos::WorkerActor::ensureBufferManager()",
        Poco::format("%s[%s] has no buffer manager set", block->getName(), name));
}

/***********************************************************************
 * port subscribe/unsubscribe
 **********************************************************************/
void Pothos::WorkerActor::subscribeInput(const std::string &action, const std::string &myPortName, Pothos::InputPort *inputPort)
{
    ActorInterfaceLock lock(this);

    //locate the subscriber in the list
    auto &subscribers = this->outputs.at(myPortName)->_subscribers;
    auto it = std::find(subscribers.begin(), subscribers.end(), inputPort);
    const bool found = it != subscribers.end();

    if (action == "add") //add to the output port's subscribers list
    {
        if (found) throw PortAccessError("Pothos::WorkerActor::subscribePort()",
            Poco::format("input %s subscription exists in output port %s", inputPort->name(), myPortName));
        subscribers.push_back(inputPort);
    }
    if (action == "remove") //remove from the output port's subscribers list
    {
        if (not found) throw PortAccessError("Pothos::WorkerActor::unsubscribePort()",
            Poco::format("input %s subscription missing from output port %s", inputPort->name(), myPortName));
        subscribers.erase(it);
    }

    //empty subscribers, don't hold onto the buffer manager so it can be cleaned up
    if (subscribers.empty()) bufferManagerTmpCache[false][myPortName].reset();

    //when unsubscripted, ensure that we have a local buffer manager
    if (subscribers.empty()) this->ensureOutputBufferManagerNoLock(myPortName);

    this->updatePorts();
}

void Pothos::WorkerActor::subscribeOutput(const std::string &action, const std::string &myPortName, Pothos::OutputPort *outputPort)
{
    ActorInterfaceLock lock(this);

    //locate the subscriber in the list
    auto &subscribers = this->inputs.at(myPortName)->_subscribers;
    auto it = std::find(subscribers.begin(), subscribers.end(), outputPort);
    const bool found = it != subscribers.end();

    if (action == "add") //add to the input port's subscribers list
    {
        if (found) throw PortAccessError("Pothos::WorkerActor::subscribePort()",
            Poco::format("output %s subscription exists in input port %s", outputPort->name(), myPortName));
        subscribers.push_back(outputPort);
    }
    if (action == "remove") //remove from the input port's subscribers list
    {
        if (not found) throw PortAccessError("Pothos::WorkerActor::unsubscribePort()",
            Poco::format("output %s subscription missing from input port %s", outputPort->name(), myPortName));
        subscribers.erase(it);
    }

    //empty subscribers, don't hold onto the buffer manager so it can be cleaned up
    if (subscribers.empty()) bufferManagerTmpCache[true][myPortName].reset();

    this->updatePorts();
}

/***********************************************************************
 * activate/deactivate
 **********************************************************************/
void Pothos::WorkerActor::setActiveStateOn(void)
{
    ActorInterfaceLock lock(this);

    //ensure that every output port gets a buffer manager
    //because connection logic skips over unused ports
    for (const auto &entry : this->outputs)
    {
        this->ensureOutputBufferManagerNoLock(entry.first);
    }

    POTHOS_EXCEPTION_TRY
    {
        this->activeState = true;
        this->block->activate();
        this->activityIndicator.fetch_add(1, std::memory_order_relaxed);
    }
    POTHOS_EXCEPTION_CATCH(const Exception &ex)
    {
        this->activeState = false;
        throw ex;
    }
}

void Pothos::WorkerActor::setActiveStateOff(void)
{
    ActorInterfaceLock lock(this);

    //not activated? just return
    if (not this->activeState) return;

    this->activeState = false;
    this->block->deactivate();
    this->activityIndicator.fetch_add(1, std::memory_order_relaxed);
}

/***********************************************************************
 * work task dispatcher
 **********************************************************************/
void Pothos::WorkerActor::workTask(void)
{
    if (not activeState) return;
    this->numTaskCalls++;
    TimeAccumulator taskTime(this->totalTimeTask);

    //prework
    {
        TimeAccumulator preWorkTime(this->totalTimePreWork);
        if (not this->preWorkTasks()) return;
    }

    //work
    POTHOS_EXCEPTION_TRY
    {
        this->numWorkCalls++;
        TimeAccumulator workTime(this->totalTimeWork);
        block->work();
    }
    POTHOS_EXCEPTION_CATCH(const Exception &ex)
    {
        poco_error_f2(Poco::Logger::get("Pothos.Block.work"), "%s: %s", block->getName(), ex.displayText());
    }

    //postwork
    {
        TimeAccumulator preWorkTime(this->totalTimePostWork);
        this->postWorkTasks();
    }

    this->timeLastWork = std::chrono::high_resolution_clock::now();
}

/***********************************************************************
 * call slots
 **********************************************************************/
void Pothos::WorkerActor::handleSlotCalls(InputPort &port)
{
    while (not port.slotCallsEmpty())
    {
        POTHOS_EXCEPTION_TRY
        {
            const ObjectVector args = port.slotCallsPop();
            block->opaqueCallHandler(port.name(), args.data(), args.size());
            this->flagInternalChange();
            this->activityIndicator.fetch_add(1, std::memory_order_relaxed);
        }
        POTHOS_EXCEPTION_CATCH(const Exception &ex)
        {
            poco_error_f3(Poco::Logger::get("Pothos.Block.callSlot"), "%s[%s]: %s", block->getName(), port.alias(), ex.displayText());
        }
    }
}

/***********************************************************************
 * pre-work
 **********************************************************************/
bool Pothos::WorkerActor::preWorkTasks(void)
{
    const size_t BIG = (1 << 30);

    //////////////// output state calculation ///////////////////
    block->_workInfo.minOutElements = BIG;
    block->_workInfo.minAllOutElements = BIG;
    for (const auto &entry : this->outputs)
    {
        auto &port = *entry.second;
        port._workEvents = 0;

        //an empty token manager means that upstream blocks
        //hold all of our message resources, we can't continue
        if (port.tokenManagerEmpty()) return false;

        //signal ports don't use buffers, skip the code below
        if (port.isSignal()) continue;

        //is it ok to use the read-before-write optimization?
        const auto tryRBW = port._readBeforeWritePort != nullptr and
        port.dtype().size() == port._readBeforeWritePort->dtype().size();
        if (tryRBW)
        {
            port._readBeforeWritePort->_buffer.clear();
            port._readBeforeWritePort->bufferAccumulatorFront(port._buffer);
        }

        //now determine the buffer provided to this port
        if (tryRBW and port._buffer.useCount() == 2 and //2 -> accumulator + this port
            port._buffer.getEnd() <= port._buffer.getBuffer().getEnd()) //no amalgamation
        {
            port._bufferFromManager = false;
        }
        else if (port.bufferManagerEmpty())
        {
            port._buffer.clear();
            port._bufferFromManager = false;
        }
        else
        {
            port.bufferManagerFront(port._buffer);
            port._bufferFromManager = true;
        }
        port._buffer.dtype = port.dtype(); //always copy from port's dtype setting
        port._elements = port._buffer.elements();
        if (port._elements == 0) return false;
        port._pendingElements = 0;
        if (port.index() != -1)
        {
            assert(block->_workInfo.outputPointers.size() > size_t(port.index()));
            block->_workInfo.outputPointers[port.index()] = port._buffer.as<void *>();
            block->_workInfo.minOutElements = std::min(block->_workInfo.minOutElements, port._elements);
        }
        block->_workInfo.minAllOutElements = std::min(block->_workInfo.minAllOutElements, port._elements);
    }

    //////////////// input state calculation ///////////////////
    bool hasBufferedPorts = false;
    bool reserveReached = false;
    bool hasInputMessage = false;
    block->_workInfo.minInElements = BIG;
    block->_workInfo.minAllInElements = BIG;
    for (const auto &entry : this->inputs)
    {
        auto &port = *entry.second;
        port._workEvents = 0;
        if (port.isSlot())
        {
            this->handleSlotCalls(port);
            continue;
        }
        hasBufferedPorts = true;

        //perform minimum reserve accumulator require to recover from possible element fragmentation
        const size_t requireElems = std::max<size_t>(1, port._reserveElements);
        port.bufferAccumulatorRequire(requireElems*port.dtype().size());
        port.bufferAccumulatorFront(port._buffer);
        port._elements = port._buffer.length/port.dtype().size();
        if (port._elements >= port._reserveElements) reserveReached = true;
        if (not port.asyncMessagesEmpty()) hasInputMessage = true;
        port._pendingElements = 0;
        port._labelIter = port._inlineMessages;
        if (port.index() != -1)
        {
            assert(block->_workInfo.inputPointers.size() > size_t(port.index()));
            block->_workInfo.inputPointers[port.index()] = port._buffer.as<const void *>();
            block->_workInfo.minInElements = std::min(block->_workInfo.minInElements, port._elements);
        }
        block->_workInfo.minAllInElements = std::min(block->_workInfo.minAllInElements, port._elements);
    }

    //calculate overall minimums
    block->_workInfo.minElements = std::min(block->_workInfo.minInElements, block->_workInfo.minOutElements);
    block->_workInfo.minAllElements = std::min(block->_workInfo.minAllInElements, block->_workInfo.minAllOutElements);

    //arbitrary time, but its small
    block->_workInfo.maxTimeoutNs = 1000000; //1 millisecond

    //perform work when:
    //1) at least one reserve was met,
    //2) or a port has an input message,
    //3) or there are buffered input ports
    return reserveReached or hasInputMessage or (not hasBufferedPorts);
}

/***********************************************************************
 * post-work
 **********************************************************************/
void Pothos::WorkerActor::postWorkTasks(void)
{
    ///////////////////// input handling ////////////////////////

    size_t inputWorkEvents = 0;

    for (const auto &entry : this->inputs)
    {
        auto &port = *entry.second;
        const size_t bytes = port._pendingElements*port.dtype().size();

        //propagate labels and delete old
        size_t numLabels = 0;
        auto &allLabels = port._inlineMessages;
        for (size_t i = 0; i < allLabels.size(); i++)
        {
            if (allLabels[i].index < port._pendingElements) numLabels++;
            //adjust labels index for new relative position
            else allLabels[i].index -= port._pendingElements;
        }

        if (numLabels != 0)
        {
            port._labelIter = LabelIteratorRange(allLabels.data(), allLabels.data()+numLabels);
            POTHOS_EXCEPTION_TRY
            {
                block->propagateLabels(&port);
            }
            POTHOS_EXCEPTION_CATCH(const Exception &ex)
            {
                poco_error_f2(Poco::Logger::get("Pothos.Block.propagateLabels"), "%s: %s", block->getName(), ex.displayText());
            }

            allLabels.erase(allLabels.begin(), allLabels.begin()+numLabels);
            port._totalLabels += numLabels;
        }

        //pop the consumed bytes from the accumulator
        if (bytes != 0)
        {
            port.bufferAccumulatorPop(bytes);
        }
        port._buffer.clear(); //clear reference

        //move consumed elements into total
        port._totalElements += port._pendingElements;
        inputWorkEvents += port._workEvents;
    }

    //react to consumption events
    if (inputWorkEvents != 0)
    {
        this->flagInternalChange();
        this->activityIndicator.fetch_add(1, std::memory_order_relaxed);
        this->timeLastConsumed = std::chrono::high_resolution_clock::now();
    }

    ///////////////////// output handling ////////////////////////
    //Note: output buffer production must come after propagateLabels()

    size_t outputWorkEvents = 0;

    for (const auto &entry : this->outputs)
    {
        auto &port = *entry.second;
        size_t elemsDequeued = 0;
        size_t bytesDequeued = 0;

        //set the buffer length, send it, pop from manager, clear reference
        const size_t pendingBytes = port._pendingElements*port.buffer().dtype.size();
        if (pendingBytes != 0)
        {
            auto &buffer = port._buffer;
            buffer.length = pendingBytes;
            if (port._bufferFromManager)
            {
                if (buffer.length > buffer.getBuffer().getLength())
                {
                    poco_error_f4(Poco::Logger::get("Pothos.Block.produce"), "%s[%s] overproduced %z bytes, %z available",
                        block->getName(), port.alias(), buffer.length, buffer.getBuffer().getLength());
                }

                //Some buffer managers may reuse the remainder of the buffer based on how much is left.
                //The reserve logic disables this optimization by removing the entire available buffer
                //when its remainder will not meet the reserve requirement. Circular buffers are exempt
                //here because they will auto-accumulate larger as resources return from downstream.
                else if (port._reserveElements != 0 and buffer.getAlias() == 0 and //non-circular buffer
                    port._elements - port._pendingElements < port._reserveElements) //remainder < reserve
                {
                    port.bufferManagerPop(buffer.getBuffer().getLength());
                }

                else port.bufferManagerPop(buffer.length);
            }
            port.postBuffer(buffer);
        }

        //Outside of produce, the block may use popElements() or increase the reserve.
        //For these reasons, the reserve logic must be implemented for this case as well.
        //Because popElements() could have changed the available length of the front buffer,
        //we re-acquire the current front buffer and check its length against the reserve.
        else if (port._workEvents != 0 and port._reserveElements != 0 and port._buffer.getAlias() == 0)
        {
            BufferChunk buffer; port.bufferManagerFront(buffer);
            if (buffer.length != 0 and buffer.length < port._reserveElements*port._dtype.size())
            {
                port.bufferManagerPop(buffer.length);
            }
        }

        port._buffer.clear(); //clear reference

        //sort the posted labels in case the user posted out of order
        auto &postedLabels = port._postedLabels;
        auto &postedBuffers = port._postedBuffers;
        if (not postedLabels.empty()) std::sort(postedLabels.begin(), postedLabels.end());

        //send the outgoing labels with buffers
        if (not postedLabels.empty() or not postedBuffers.empty())
        {
            for (const auto &subscriber : port._subscribers)
            {
                subscriber->bufferLabelPush(postedLabels, postedBuffers);
            }
        }

        //clear posted labels
        postedLabels.clear();

        //clear posted buffers and save stats
        while (not postedBuffers.empty())
        {
            auto &buffer = postedBuffers.front();
            elemsDequeued += buffer.elements();
            bytesDequeued += buffer.length;
            postedBuffers.pop_front();
        }

        //add produced bytes into total
        port._totalElements += elemsDequeued;
        outputWorkEvents += port._workEvents;
    }

    //react to production events
    if (outputWorkEvents != 0)
    {
        this->flagInternalChange();
        this->activityIndicator.fetch_add(1, std::memory_order_relaxed);
        this->timeLastProduced = std::chrono::high_resolution_clock::now();
    }
}

std::string Pothos::WorkerActor::queryWorkStats(void)
{
    ActorInterfaceLock lock(this);
    json stats;

    //load the work stats
    stats["blockName"] = block->getName();
    stats["numTaskCalls"] = this->numTaskCalls;
    stats["numWorkCalls"] = this->numWorkCalls;
    stats["totalTimeTask"] = this->totalTimeTask.count();
    stats["totalTimeWork"] = this->totalTimeWork.count();
    stats["totalTimePreWork"] = this->totalTimePreWork.count();
    stats["totalTimePostWork"] = this->totalTimePostWork.count();
    stats["timeLastConsumed"] = this->timeLastConsumed.time_since_epoch().count();
    stats["timeLastProduced"] = this->timeLastProduced.time_since_epoch().count();
    stats["timeLastWork"] = this->timeLastWork.time_since_epoch().count();
    stats["timeStatsQuery"] = std::chrono::high_resolution_clock::now().time_since_epoch().count();

    //resolution period ratio tells the consumer how to interpret the tick counts
    stats["tickRatioNum"] = std::chrono::high_resolution_clock::period::num;
    stats["tickRatioDen"] = std::chrono::high_resolution_clock::period::den;

    //load the input port stats
    json inputStats;
    for (const auto &name : block->inputPortNames())
    {
        auto &port = *this->inputs.at(name);
        if (port.isSlot()) continue;
        json portStats;
        portStats["totalElements"] = port.totalElements();
        portStats["totalBuffers"] = port.totalBuffers();
        portStats["totalLabels"] = port.totalLabels();
        portStats["totalMessages"] = port.totalMessages();
        portStats["dtypeSize"] = port.dtype().size();
        portStats["dtypeMarkup"] = port.dtype().toMarkup();
        portStats["portName"] = name;
        portStats["portAlias"] = port.alias();
        portStats["reserveElements"] = port._reserveElements;
        {
            BufferChunk frontBuff; port.bufferAccumulatorFront(frontBuff);
            portStats["frontBytes"] = frontBuff.length;
        }
        {
            std::lock_guard<Util::SpinLock> lockB(port._bufferAccumulatorLock);
            portStats["enqueuedBytes"] = port._bufferAccumulator.getTotalBytesAvailable();
            portStats["enqueuedBuffers"] = port._bufferAccumulator.getUniqueManagedBufferCount();
            portStats["enqueuedLabels"] = port._inlineMessages.size()+port._inputInlineMessages.size();
        }
        {
            std::lock_guard<Util::SpinLock> lockM(port._asyncMessagesLock);
            portStats["enqueuedMessages"] = port._asyncMessages.size();
        }
        inputStats.push_back(portStats);
    }
    if (not inputStats.empty()) stats["inputStats"] = inputStats;

    //load the output port stats
    json outputStats;
    for (const auto &name : block->outputPortNames())
    {
        auto &port = *this->outputs.at(name);
        if (port.isSignal()) continue;
        json portStats;
        portStats["totalElements"] = port.totalElements();
        portStats["totalBuffers"] = port.totalBuffers();
        portStats["totalLabels"] = port.totalLabels();
        portStats["totalMessages"] = port.totalMessages();
        portStats["dtypeSize"] = port.dtype().size();
        portStats["dtypeMarkup"] = port.dtype().toMarkup();
        portStats["portName"] = name;
        portStats["portAlias"] = port.alias();
        {
            BufferChunk frontBuff; port.bufferManagerFront(frontBuff);
            portStats["frontBytes"] = frontBuff.length;
        }
        portStats["tokensEmpty"] = port.tokenManagerEmpty();
        outputStats.push_back(portStats);
    }
    if (not outputStats.empty()) stats["outputStats"] = outputStats;

    return stats.dump();
}

#include <Pothos/Managed.hpp>

static auto managedWorkerActor = Pothos::ManagedClass()
    .registerClass<Pothos::WorkerActor>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setActiveStateOn))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setActiveStateOff))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, subscribeInput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, subscribeOutput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getBufferMode))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, getBufferManager))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, setOutputBufferManager))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoAllocateInput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoAllocateOutput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoDeleteInput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, autoDeleteOutput))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, queryActivityIndicator))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::WorkerActor, queryWorkStats))
    .commit("Pothos/WorkerActor");
