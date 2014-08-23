//
// Callable/CallInterfaceImpl.hpp
//
// Template implementation details for CallInterface.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/CallInterface.hpp>
#include <Pothos/Callable/Exception.hpp>
#include <Pothos/Object/ObjectImpl.hpp>
#include <utility> //std::forward

namespace Pothos {

#for $NARGS in range($MAX_ARGS)
/***********************************************************************
 * Templated call gateways with $(NARGS) args
 **********************************************************************/
template <typename ReturnType, $expand('typename A%d', $NARGS)>
ReturnType CallInterface::call($expand('A%d &&a%d', $NARGS)) const
{
    Object r = this->callObject($expand('std::forward<A%d>(a%d)', $NARGS));
    try
    {
        return r.convert<ReturnType>();
    }
    catch(const Exception &ex)
    {
        throw CallableReturnError("Pothos::Callable::call()", ex);
    }
}

#cond $NARGS > 0
template <$expand('typename A%d', $NARGS)>
Object CallInterface::callObject($expand('A%d &&a%d', $NARGS)) const
{
    Object args[$(max(1, $NARGS))];
    #for $i in range($NARGS):
    args[$i] = Object::make(std::forward<A$i>(a$i));
    #end for
    return this->opaqueCall(args, $NARGS);
}

template <$expand('typename A%d', $NARGS)>
void CallInterface::callVoid($expand('A%d &&a%d', $NARGS)) const
{
    this->callObject($expand('std::forward<A%d>(a%d)', $NARGS));
}
#end if

#end for
} //namespace Pothos
