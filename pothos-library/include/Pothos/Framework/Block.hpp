//
// Framework/Block.hpp
//
// This file contains the interface for creating custom Blocks.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Util/UID.hpp>
#include <Pothos/Framework/CallRegistry.hpp>
#include <Pothos/Framework/WorkInfo.hpp>
#include <Pothos/Framework/InputPort.hpp>
#include <Pothos/Framework/OutputPort.hpp>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace Theron {
    class Framework;
} //namespace Theron

namespace Pothos {

/*!
 * Block is an interface for creating custom computational processing.
 * Users should subclass Block, setup the input and output ports,
 * and overload the work() method for a custom computational task.
 *
 * The outputs of a Block can be connected to the inputs of another.
 * Any resources produced at the Block's output ports will be
 * make available to the other Block's connected input ports.
 */
class POTHOS_API Block : protected CallRegistry, public Util::UID
{
public:

    //! Default constructor
    explicit Block(void);

    //! Virtual destructor
    virtual ~Block(void);

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
     * \param input a pointer to the input port with labels
     * \param labels the labels within the consumed region
     */
    virtual void propagateLabels(const InputPort *input, const LabelIteratorRange &labels);

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

public:

    /*!
     * Get the names of the input ports in the order they were allocated.
     */
    std::vector<std::string> inputPortNames(void);

    /*!
     * Get the names of the output ports in the order they were allocated.
     */
    std::vector<std::string> outputPortNames(void);

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
     * Configure an input port with the given data type.
     */
    void setupInput(const std::string &name, const DType &dtype = "byte");

    /*!
     * Configure an input port with the given data type.
     */
    void setupInput(const size_t index, const DType &dtype);

    /*!
     * Configure an output port with the given data type.
     */
    void setupOutput(const std::string &name, const DType &dtype = "byte");

    /*!
     * Configure an output port with the given data type.
     */
    void setupOutput(const size_t index, const DType &dtype);

    /*!
     * Export a function call on this block to set/get parameters.
     */
    void registerCallable(const std::string &name, const Callable &call);

private:
    WorkInfo _workInfo;
    std::vector<std::string> _inputPortNames;
    std::vector<std::string> _outputPortNames;
    std::vector<InputPort*> _indexedInputs;
    std::vector<OutputPort*> _indexedOutputs;
    std::map<std::string, InputPort*> _namedInputs;
    std::map<std::string, OutputPort*> _namedOutputs;
    std::map<std::string, std::unique_ptr<InputPort>> _inputs;
    std::map<std::string, std::unique_ptr<OutputPort>> _outputs;
    std::map<std::string, Callable> _calls;
    bool _activeState;
    std::shared_ptr<Theron::Framework> _framework;
public:
    std::shared_ptr<WorkerActor> _actor;
    friend class WorkerActor;
    Object opaqueCall(const std::string &name, const Object *inputArgs, const size_t numArgs);
};

} //namespace Pothos
