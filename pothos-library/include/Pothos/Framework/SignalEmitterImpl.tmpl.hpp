//
// Framework/SignalEmitterImpl.hpp
//
// Template method implementations for SignalEmitter.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/SignalEmitter.hpp>

namespace Pothos {

#for $NARGS in range(1, $MAX_ARGS)
template <$expand('typename A%d', $NARGS)>
void SignalEmitter::emitSignal(const std::string &name, $expand('A%d &&a%d', $NARGS))
{
    std::vector<Object> args($(max(1, $NARGS)));
    #for $i in range($NARGS):
    args[$i] = Object::make(std::forward<A$i>(a$i));
    #end for
    this->emitSignalArgs(name, args);
}

#end for

} //namespace Pothos
