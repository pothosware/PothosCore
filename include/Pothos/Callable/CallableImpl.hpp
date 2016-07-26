///
/// \file Callable/CallableImpl.hpp
///
/// Template implementation details for Callable.
///
/// \copyright
/// Copyright (c) 2013-2016 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/Callable.hpp>
#include <Pothos/Callable/CallInterfaceImpl.hpp>
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
    return this->bind(Object(std::forward<ValueType>(val)), argNo);
}

namespace Detail {

/***********************************************************************
 * Function specialization for return type with variable args
 **********************************************************************/
template <typename ReturnType, typename... ArgsType>
class CallableFunctionContainer : public Detail::CallableContainer
{
public:
    template <typename FcnType>
    CallableFunctionContainer(const FcnType &fcn):
        _fcn(fcn)
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return sizeof...(ArgsType);
    }

    const std::type_info &type(const int argNo)
    {
        return typeR<ArgsType..., ReturnType>(argNo);
    }

    Object call(const Object *args)
    {
        return call(args, typename Gens<sizeof...(ArgsType)>::Type());
    }

private:

    //! implement recursive type() tail-case
    template <typename T>
    const std::type_info &typeR(const int argNo)
    {
        if (argNo == 0) return typeid(T);
        return typeid(ReturnType);
    }

    //! implement recursive type() for pack iteration
    template <typename T0, typename T1, typename... Ts>
    const std::type_info &typeR(const int argNo)
    {
        if (argNo == 0) return typeid(T0);
        return typeR<T1, Ts...>(argNo-1);
    }

    //! sequence generator used to help index object arguments below
    template<int...> struct Seq {};
    template<int N, int... S> struct Gens : Gens<N-1, N-1, S...> {};
    template<int... S> struct Gens<0, S...>{ typedef Seq<S...> Type; };

    //! implement call() using a generator sequence to handle array access
    template<int ...S>
    Object call(const Object *args, Seq<S...>)
    {
        checkArgs(args); //fixes warning for 0 args case
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, args[S].extract<ArgsType>()...);
    }

    //! NOP call used to avoid warning above
    void checkArgs(const Object *){}

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const ArgsType&... args)
        {
            return Object::make(fcn(args...));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const ArgsType&... args)
        {
            fcn(args...); return Object();
        }
    };

    std::function<ReturnType(ArgsType...)> _fcn;
};

template <typename ClassType, typename... ArgsType>
ClassType CallableFactoryWrapper(const ArgsType&... args)
{
    return ClassType(args...);
}

template <typename ClassType, typename... ArgsType>
ClassType *CallableFactoryNewWrapper(const ArgsType&... args)
{
    return new ClassType(args...);
}

template <typename ClassType, typename... ArgsType>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const ArgsType&... args)
{
    return std::shared_ptr<ClassType>(new ClassType(args...));
}

} //namespace Detail

/***********************************************************************
 * Templated factory/constructor calls with variable args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename... ArgsType>
Callable::Callable(ReturnType(ClassType::*fcn)(ArgsType...)):
    _impl(new Detail::CallableFunctionContainer<ReturnType, ClassType &, ArgsType...>(std::mem_fn(fcn)))
{
    return;
}

template <typename ReturnType, typename ClassType, typename... ArgsType>
Callable::Callable(ReturnType(ClassType::*fcn)(ArgsType...) const):
    _impl(new Detail::CallableFunctionContainer<ReturnType, const ClassType &, ArgsType...>(std::mem_fn(fcn)))
{
    return;
}

template <typename ReturnType, typename... ArgsType>
Callable::Callable(ReturnType(*fcn)(ArgsType...)):
    _impl(new Detail::CallableFunctionContainer<ReturnType, ArgsType...>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename... ArgsType>
Callable Callable::make(ReturnType(ClassType::*fcn)(ArgsType...))
{
    return Callable(fcn);
}

template <typename ReturnType, typename ClassType, typename... ArgsType>
Callable Callable::make(ReturnType(ClassType::*fcn)(ArgsType...) const)
{
    return Callable(fcn);
}

template <typename ReturnType, typename... ArgsType>
Callable Callable::make(ReturnType(*fcn)(ArgsType...))
{
    return Callable(fcn);
}

template <typename ClassType, typename... ArgsType>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, ArgsType...>);
}

template <typename ClassType, typename... ArgsType>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, ArgsType...>);
}

template <typename ClassType, typename... ArgsType>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, ArgsType...>);
}

} //namespace Pothos
