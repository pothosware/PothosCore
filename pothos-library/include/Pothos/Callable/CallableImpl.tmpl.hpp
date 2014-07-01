//
// Callable/CallableImpl.hpp
//
// Template implementation details for Callable.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/Callable.hpp>
#include <Pothos/Callable/Exception.hpp>
#include <Pothos/Object/ObjectImpl.hpp>
#include <functional> //std::function
#include <type_traits> //std::type_info, std::is_void
#include <utility> //std::forward

namespace Pothos {
namespace Detail {

struct POTHOS_API CallableContainer
{
    CallableContainer(void);
    virtual ~CallableContainer(void);
    virtual size_t getNumArgs(void) const = 0;
    virtual const std::type_info &type(const int argNo) = 0;
    virtual Object call(const Object *args) = 0;
};

} //namespace Detail

template <typename ValueType>
Callable &Callable::bind(ValueType &&val, const size_t argNo)
{
    return this->bind(Object::make(std::forward<ValueType>(val)), argNo);
}

namespace Detail {
#for $NARGS in range($MAX_ARGS+1)
/***********************************************************************
 * Function specialization for return type with $(NARGS) args
 **********************************************************************/
template <typename ReturnType, $expand('typename A%d', $NARGS)>
struct CallableFunctionContainer$(NARGS) : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer$(NARGS)(const FcnType &fcn):
        _fcn(std::bind(fcn, $expand('std::placeholders::_$(%d+1)', $NARGS)))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return $NARGS;
    }

    const std::type_info &type(const int $optarg('argNo', $NARGS))
    {
        #for $i in range($NARGS):
        if (argNo == $i) return typeid(A$i);
        #end for
        return typeid(ReturnType);
    }

    Object call(const Object * $optarg('args', $NARGS))
    {
        #for $i in range($NARGS):
        auto &a$i = args[$i].extract<A$i>();
        #end for
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, $expand('a%d', $NARGS));
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, $expand('const A%d &a%d', $NARGS))
        {
            return Object::make(fcn($expand('a%d', $NARGS)));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, $expand('const A%d &a%d', $NARGS))
        {
            fcn($expand('a%d', $NARGS)); return Object();
        }
    };

    std::function<ReturnType($expand('A%d', $NARGS))> _fcn;
};

template <typename ClassType, $expand('typename A%d', $NARGS)>
ClassType CallableFactoryWrapper($expand('const A%d &a%d', $NARGS))
{
    return ClassType($expand('a%d', $NARGS));
}

template <typename ClassType, $expand('typename A%d', $NARGS)>
ClassType *CallableFactoryNewWrapper($expand('const A%d &a%d', $NARGS))
{
    return new ClassType($expand('a%d', $NARGS));
}

template <typename ClassType, $expand('typename A%d', $NARGS)>
std::shared_ptr<ClassType> CallableFactorySharedWrapper($expand('const A%d &a%d', $NARGS))
{
    return std::shared_ptr<ClassType>(new ClassType($expand('a%d', $NARGS)));
}
#end for

} //namespace Detail

#for $NARGS in range($MAX_ARGS)
/***********************************************************************
 * Templated factory/constructor calls with $(NARGS) args
 **********************************************************************/
template <typename ReturnType, typename ClassType, $expand('typename A%d', $NARGS)>
Callable::Callable(ReturnType(ClassType::*fcn)($expand('A%d', $NARGS))):
    _impl(new Detail::CallableFunctionContainer$(NARGS+1)<ReturnType, ClassType &, $expand('A%d', $NARGS)>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, $expand('typename A%d', $NARGS)>
Callable::Callable(ReturnType(ClassType::*fcn)($expand('A%d', $NARGS)) const):
    _impl(new Detail::CallableFunctionContainer$(NARGS+1)<ReturnType, const ClassType &, $expand('A%d', $NARGS)>(fcn))
{
    return;
}

template <typename ReturnType, $expand('typename A%d', $NARGS)>
Callable::Callable(ReturnType(*fcn)($expand('A%d', $NARGS))):
    _impl(new Detail::CallableFunctionContainer$(NARGS)<ReturnType, $expand('A%d', $NARGS)>(fcn))
{
    return;
}

template <$expand('typename A%d', $NARGS), typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)($expand('A%d', $NARGS)))
{
    return Callable(fcn);
}

template <$expand('typename A%d', $NARGS), typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)($expand('A%d', $NARGS)) const)
{
    return Callable(fcn);
}

template <$expand('typename A%d', $NARGS), typename ReturnType>
Callable Callable::make(ReturnType(*fcn)($expand('A%d', $NARGS)))
{
    return Callable(fcn);
}

template <typename ClassType, $expand('typename A%d', $NARGS)>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, $expand('A%d', $NARGS)>);
}

template <typename ClassType, $expand('typename A%d', $NARGS)>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, $expand('A%d', $NARGS)>);
}

template <typename ClassType, $expand('typename A%d', $NARGS)>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, $expand('A%d', $NARGS)>);
}

/***********************************************************************
 * Templated call gateways with $(NARGS) args
 **********************************************************************/
template <typename ReturnType, $expand('typename A%d', $NARGS)>
ReturnType Callable::call($expand('A%d &&a%d', $NARGS)) const
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
Object Callable::callObject($expand('A%d &&a%d', $NARGS)) const
{
    Object args[$(max(1, $NARGS))];
    #for $i in range($NARGS):
    args[$i] = Object::make(std::forward<A$i>(a$i));
    #end for
    return this->opaqueCall(args, $NARGS);
}

template <$expand('typename A%d', $NARGS)>
void Callable::callVoid($expand('A%d &&a%d', $NARGS)) const
{
    this->callObject($expand('std::forward<A%d>(a%d)', $NARGS));
}
#end if

#end for

} //namespace Pothos
