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

    /*!
     * Emit a signal with 0 args.
     */
    inline
    void emitSignal(const std::string &name);

    /*!
     * Emit a signal with 1 args.
     */
    template <typename A0>
    void emitSignal(const std::string &name, const A0 &a0);

    /*!
     * Emit a signal with 2 args.
     */
    template <typename A0, typename A1>
    void emitSignal(const std::string &name, const A0 &a0, const A1 &a1);

    /*!
     * Emit a signal with 3 args.
     */
    template <typename A0, typename A1, typename A2>
    void emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2);

    /*!
     * Emit a signal with 4 args.
     */
    template <typename A0, typename A1, typename A2, typename A3>
    void emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3);

    /*!
     * Emit a signal with 5 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    void emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4);

    /*!
     * Emit a signal with 6 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    void emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5);

    /*!
     * Emit a signal with 7 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    void emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6);

    /*!
     * Emit a signal with 8 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    void emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7);

    /*!
     * Emit a signal with 9 args.
     */
    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    void emitSignal(const std::string &name, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8);

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
