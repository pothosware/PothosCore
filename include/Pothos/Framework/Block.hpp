///
/// \file Framework/Block.hpp
///
/// This file contains the interface for creating custom Blocks.
///
/// \copyright
/// Copyright (c) 2014-2019 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/Connectable.hpp>
#include <Pothos/Framework/WorkInfo.hpp>
#include <Pothos/Framework/InputPort.hpp>
#include <Pothos/Framework/OutputPort.hpp>
#include <Pothos/Framework/ThreadPool.hpp>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace Pothos {

    class BufferManager;

/*!
 * Block is an interface for creating custom computational processing.
 * Users should subclass Block, setup the input and output ports,
 * and overload the work() method for a custom computational task.
 *
 * The outputs of a Block can be connected to the inputs of another.
 * Any resources produced at the Block's output ports will be
 * make available to the other Block's connected input ports.
 */
class POTHOS_API Block : public Connectable
{
public:

    //! Default constructor
    Block(void);

    //! Virtual destructor
    virtual ~Block(void);

    //! Set the thread pool used by this block.
    void setThreadPool(const ThreadPool &threadPool);

    //! Get the thread pool used by this block
    const ThreadPool &getThreadPool(void) const;

protected:
    /*!
     * The work() method, called when resources are available.
     * Subclasses must override this call when creating a worker.
     *
     * When work() is invoked, the user's code can access the port,
     * consume input resources, and produce output resources.
     *
     * Only the work() thread is allowed to call this method,
     * therefore users should never directly invoke this method.
     */
    virtual void work(void);

    /*!
     * The activate() method, called when the topology execution begins.
     * Override this call to implement a custom topology activation hook.
     */
    virtual void activate(void);

    /*!
     * The deactivate() method, called when the topology execution ends.
     * Override this call to implement a custom topology deactivation hook.
     */
    virtual void deactivate(void);

    /*!
     * The work() thread calls the propagateLabels() method after work()
     * when labels are available to propagate to downstream consumers.
     *
     * Default behavior: All labels with an index less than the number
     * of consumed elements will be propagated to all output ports.
     * Subclasses may override this call to customize its behavior.
     *
     * Only the work() thread is allowed to call this method,
     * therefore users should never directly invoke this method.
     *
     * Access the labels iterator with the call to input->labels().
     * This iterator will contain only labels from the consumed elements.
     * Forward labels to the output ports using postLabel() on an output port object.
     *
     * \param input a pointer to the input port with labels
     */
    virtual void propagateLabels(const InputPort *input);

    /*!
     * The opaque call handler handles dispatching calls to registered methods.
     * The user may overload this call to install their own custom handler.
     * \throws BlockCallNotFound when no call registered for the provided name
     * \throws Exception when the registered call itself throws an exception
     * \param name the name of a call registered to this Block with registerCall()
     * \param inputArgs an array of input arguments wrapped in type Object
     * \param numArgs the number of arguments in the array inputArgs
     * \return the result of making the registered call, wrapped in type Object
     */
    virtual Object opaqueCallHandler(const std::string &name, const Object *inputArgs, const size_t numArgs);

    /*!
     * Get a buffer manager for this input port.
     * The user may overload this call to install a custom buffer manager.
     *
     * The domain parameter describes the memory used by the upstream blocks.
     * Knowing the domain allows the implementer of getInputBufferManager to
     *   - abdicate to the upstream's buffer managers (null return)
     *   - provide a replacement upstream buffer manager (return manager)
     *   - protest the ability to interact with the domain (throw exception)
     *
     * \throws PortDomainError when the domain is incompatible
     * \param name the name of an input port on this block
     * \param domain the domain of the upstream blocks
     * \return a new buffer manager for this port or null sptr
     */
    virtual std::shared_ptr<BufferManager> getInputBufferManager(const std::string &name, const std::string &domain);

    /*!
     * Get a buffer manager for this output port.
     * The user may overload this call to install a custom buffer manager.
     *
     * The domain parameter describes the memory used by the downstream blocks.
     * Knowing the domain allows the implementer of getOutputBufferManager to
     *   - abdicate to the downstream's buffer managers (null return)
     *   - provide a replacement downstream buffer manager (return manager)
     *   - protest the ability to interact with the domain (throw exception)
     *
     * \throws PortDomainError when the domain is incompatible
     * \param name the name of an output port on this block
     * \param domain the domain of the downstream blocks
     * \return a new buffer manager for this port or null sptr
     */
    virtual std::shared_ptr<BufferManager> getOutputBufferManager(const std::string &name, const std::string &domain);

public:

    /*!
     * The prepare() method allows the block to say whether it will be able to
     * do any work, regardless of whether input or output is available. For
     * example, resources unknown to the framework may be unavailable. This call
     * happens before the framework determines whether it thinks there is work
     * for the block.
     *
     * The block should not access framework-managed (e.g., ports, messages,
     * signals/slots) in this call, though it is free to do other things.
     *
     * Returns true if the framework should proceed normally and determine
     * whether work is available, false if the block would be unable to run and
     * should be passed over.
     */
    virtual bool prepare(void);

    /*!
     * Set the displayable alias for the specified input port.
     */
    void setInputAlias(const std::string &portName, const std::string &alias);

    /*!
     * Set the displayable alias for the specified output port.
     */
    void setOutputAlias(const std::string &portName, const std::string &alias);

    /*!
     * Get a vector of info about all of the input ports available.
     */
    std::vector<PortInfo> inputPortInfo(void);

    /*!
     * Get a vector of info about all of the output ports available.
     */
    std::vector<PortInfo> outputPortInfo(void);

    /*!
     * Get the input port at the specified port name.
     */
    InputPort *input(const std::string &name) const;

    /*!
     * Get the input port at the specified port index.
     */
    InputPort *input(const size_t index) const;

    /*!
     * Get the output port at the specified port name.
     */
    OutputPort *output(const std::string &name) const;

    /*!
     * Get the output port at the specified port index.
     */
    OutputPort *output(const size_t index) const;

    /*!
     * Get the indexable input ports.
     * These ports have a port name which is an integer.
     * Indexable ports can be accessed with O(1) access time.
     * \return a vector of pointers to input port objects
     */
    const std::vector<InputPort*> &inputs(void) const;

    /*!
     * Get the indexable output ports.
     * These ports have a port name which is an integer.
     * Indexable ports can be accessed with O(1) access time.
     * \return a vector of pointers to output port objects
     */
    const std::vector<OutputPort*> &outputs(void) const;

    /*!
     * Get all input ports.
     * These ports can be accessed with the port name string.
     * \return a map of pointers to input port objects
     */
    const std::map<std::string, InputPort*> &allInputs(void) const;

    /*!
     * Get all output ports.
     * These ports can be accessed with the port name string.
     * \return a map of pointers to output port objects
     */
    const std::map<std::string, OutputPort*> &allOutputs(void) const;

    /*!
     * Get information about a work session that is not port-specific.
     * The info is valid during calls to work() and propagateLabels().
     */
    const WorkInfo &workInfo(void) const;

    /*!
     * Is the block in an active state?
     * This is a thread-safe way for a block's methods
     * to determine if the processing is currently active.
     * \return true when the topology is executing
     */
    bool isActive(void) const;

    /*!
     * Configure an input port with the given data type.
     * The data type parameter specifies the size in bytes per input element.
     * The data type is only relevant when the port is used for streaming data.
     * The domain parameter is used to specify the type of memory consumed.
     * The domain will be passed into another block's getOutputBufferManager() call.
     * \param name the name of this input port
     * \param dtype the data type for elements
     * \param domain the expected memory domain
     * \return a pointer to the new input port
     */
    InputPort *setupInput(const std::string &name, const DType &dtype = "", const std::string &domain = "");

    /*!
     * Configure an input port with the given data type.
     * This call is equivalent to setupInput(std::to_string(index), ...);
     * \param index the index number of this input port
     * \param dtype the data type for elements
     * \param domain the expected memory domain
     * \return a pointer to the new input port
     */
    InputPort *setupInput(const size_t index, const DType &dtype = "", const std::string &domain = "");

    /*!
     * Configure an output port with the given data type.
     * The data type parameter specifies the size in bytes per output element.
     * The data type is only relevant when the port is used for streaming data.
     * The domain parameter is used to specify the type of memory produced.
     * The domain will be passed into another block's getInputBufferManager() call.
     * \param name the name of this output port
     * \param dtype the data type for elements
     * \param domain the expected memory domain
     * \return a pointer to the new output port
     */
    OutputPort *setupOutput(const std::string &name, const DType &dtype = "", const std::string &domain = "");

    /*!
     * Configure an output port with the given data type.
     * This call is equivalent to setupOutput(std::to_string(index), ...);
     * \param index the index number of this output port
     * \param dtype the data type for elements
     * \param domain the expected memory domain
     * \return a pointer to the new output port
     */
    OutputPort *setupOutput(const size_t index, const DType &dtype = "", const std::string &domain = "");

    /*!
     * Export a function call on this block to set/get parameters.
     * This call will automatically register a slot of the same name
     * when the call has a void return type and the name does not
     * start with an underscore in which case its considered private.
     * \param name the name of the callable
     * \param call the bound callable method
     */
    void registerCallable(const std::string &name, const Callable &call);

    /*!
     * Register that this block has a signal of the given name.
     * A signal is capable of emitting messages to a slot.
     * The name should not overlap with the name of an output port.
     * \param name the name of the signal
     * \return a pointer to the new output port
     */
    void registerSignal(const std::string &name);

    /*!
     * Register that this block has a slot of the given name.
     * A slot is capable of accepting messages from a signal.
     * The name should not overlap with the name of an input port.
     * Note: do not call the registerSlot function in C++,
     * as registerCallable() automatically registers a slot.
     * \param name the name of the slot
     */
    void registerSlot(const std::string &name);

    /*!
     * Register a probe given the name of a registered call.
     * A probe creates a special slot that will probe the registered call,
     * and creates a triggered signal that will emit the return value of that call.
     *
     *  - Arguments passed into the probe slot will be forwarded into the registered call.
     *  - The return value of the registered call will be passed into the first argument of the triggered signal.
     *  - If the return value of the registered call is void, then the triggered signal will have no arguments.
     *  - When not specified, the slot's name will be probe[Name], and the signal's name will be [name]Triggered.
     *
     * \param name the name of a registered call
     * \param signalName the name of the triggered signal or empty for automatic
     * \param slotName the name of the probe slot or empty for automatic
     */
    void registerProbe(
        const std::string &name,
        const std::string &signalName="",
        const std::string &slotName="");

    /*!
     * Notify the scheduler that the work() method will yeild the thread context.
     * Call this method when the work() function will not produce or consume,
     * so that the scheduler will call work() again without an external stimulus.
     * Only call this method from within a call to the work() function.
     * A typical use case for calling yield are blocks that must wait on a resource.
     * Such blocks cannot hold the thread context for more than the allowed time,
     * and must therefore return from the work() call without producing output.
     */
    void yield(void);

    /*!
     * Emit a signal to all subscribed slots.
     * \param name the name of a registered signal
     * \param args a variable number of arguments
     */
    template <typename... ArgsType>
    void emitSignal(const std::string &name, ArgsType&&... args);

    /*!
     * Call a method on a derived instance with opaque input and return types.
     * \param name the name of the method as a string
     * \param inputArgs an array of input arguments
     * \param numArgs the size of the input array
     * \return the return value as type Object
     */
    Object opaqueCallMethod(const std::string &name, const Object *inputArgs, const size_t numArgs) const;

private:
    WorkInfo _workInfo;
    std::vector<std::string> _inputPortNames;
    std::vector<std::string> _outputPortNames;
    std::vector<InputPort*> _indexedInputs;
    std::vector<OutputPort*> _indexedOutputs;
    std::map<std::string, InputPort*> _namedInputs;
    std::map<std::string, OutputPort*> _namedOutputs;
    std::multimap<std::string, Callable> _calls;
    std::map<std::string, std::pair<std::string, std::string>> _probes;
    ThreadPool _threadPool;
    Block(const Block &) = delete; // non construction-copyable
    Block &operator=(const Block &) = delete; // non copyable
public:
    std::shared_ptr<WorkerActor> _actor;
    friend class WorkerActor;
};

} //namespace Pothos
