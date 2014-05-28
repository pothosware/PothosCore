//
// Framework/SignalEmitter.hpp
//
// SignalEmitter is an interface for emitting signals in a signal+slots paradigm.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object.hpp>
#include <string>

namespace Pothos {

/*!
 * SignalEmitter is an interface for emitting signals in a signal+slots paradigm.
 */
class POTHOS_API SignalEmitter
{
public:

    //! Virtual destructor for subclassing
    virtual ~SignalEmitter(void);

    #for $NARGS in range($MAX_ARGS)
    /*!
     * Emit a signal with $NARGS args.
     */
    template <$expand('typename A%d', $NARGS)>
    void emitSignal(const std::string &name, $expand('const A%d &a%d', $NARGS));

    #end for
    /*!
     * Emit a signal given the args as an array of opaque objects.
     * \param name the name of the signal
     * \param args a pointer to an args array
     * \param numArgs the number of args in the array
     */
    virtual void emitSignalArgs(const std::string &name, const Object *args, const size_t numArgs) = 0;
};

} //namespace Pothos

inline void Pothos::SignalEmitter::emitSignal(const std::string &name)
{
    this->emitSignalArgs(name, nullptr, 0);
}
