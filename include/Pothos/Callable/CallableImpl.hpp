///
/// \file Callable/CallableImpl.hpp
///
/// Template implementation details for Callable.
///
/// \copyright
/// Copyright (c) 2013-2019 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Callable/Callable.hpp>
#include <Pothos/Callable/CallInterfaceImpl.hpp>
#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Util/Templates.hpp> //integer_sequence
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
template <typename ReturnType, typename FcnRType, typename... ArgsType>
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
        return call(args, Pothos::Util::index_sequence_for<ArgsType...>{});
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

    //! implement call() using a generator sequence to handle array access
    template<std::size_t ...S>
    Object call(const Object *args, Pothos::Util::index_sequence<S...>)
    {
        checkArgs(args); //fixes warning for 0 args case
        return CallHelper<
            decltype(_fcn),
            std::is_void<ReturnType>::value,
            std::is_same<ReturnType, FcnRType>::value,
            std::is_reference<ReturnType>::value and
            not std::is_const<typename std::remove_reference<ReturnType>::type>::value
        >::call(_fcn, args[S].extract<ArgsType>()...);
    }

    //! NOP call used to avoid warning above
    void checkArgs(const Object *){}

    //! templated call of function for optional return type
    template <typename FcnType, bool isVoid, bool isSameR, bool isReference> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false, true, false>
    {
        static Object call(const FcnType &fcn, const ArgsType&... args)
        {
            return Object::make(fcn(args...));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, false, false, false>
    {
        static Object call(const FcnType &fcn, const ArgsType&... args)
        {
            return fcn(args...);
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, false, true, true>
    {
        static Object call(const FcnType &fcn, const ArgsType&... args)
        {
            return Object::make(std::ref(fcn(args...)));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true, true, false>
    {
        static Object call(const FcnType &fcn, const ArgsType&... args)
        {
            fcn(args...); return Object();
        }
    };

    std::function<FcnRType(ArgsType...)> _fcn;
};

template <typename ClassType, typename... ArgsType>
Object CallableFactoryNewWrapper(ArgsType&&... args)
{
    return Object(new ClassType(std::forward<ArgsType>(args)...));
}

} //namespace Detail

/***********************************************************************
 * Templated factory/constructor calls with variable args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename... ArgsType>
Callable::Callable(ReturnType(ClassType::*fcn)(ArgsType...)):
    _impl(new Detail::CallableFunctionContainer<ReturnType, ReturnType, ClassType &, ArgsType...>(std::mem_fn(fcn)))
{
    return;
}

template <typename ReturnType, typename ClassType, typename... ArgsType>
Callable::Callable(ReturnType(ClassType::*fcn)(ArgsType...) const):
    _impl(new Detail::CallableFunctionContainer<ReturnType, ReturnType, const ClassType &, ArgsType...>(std::mem_fn(fcn)))
{
    return;
}

template <typename ReturnType, typename... ArgsType>
Callable::Callable(ReturnType(*fcn)(ArgsType...)):
    _impl(new Detail::CallableFunctionContainer<ReturnType, ReturnType, ArgsType...>(fcn))
{
    return;
}

template <typename ReturnType, typename... ArgsType>
Callable::Callable(const std::function<ReturnType(ArgsType...)> &fcn):
    _impl(new Detail::CallableFunctionContainer<ReturnType, ReturnType, ArgsType...>(fcn))
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

template <typename ReturnType, typename... ArgsType>
Callable Callable::make(const std::function<ReturnType(ArgsType...)> &fcn)
{
    return Callable(fcn);
}

template <typename ClassType, typename... ArgsType>
Callable Callable::factory(void)
{
    return Callable(new Detail::CallableFunctionContainer<ClassType, Object, ArgsType...>(
        &Object::emplace<ClassType, ArgsType...>));
}

template <typename ClassType, typename... ArgsType>
Callable Callable::factoryNew(void)
{
    return Callable(new Detail::CallableFunctionContainer<ClassType *, Object, ArgsType...>(
        &Detail::CallableFactoryNewWrapper<ClassType, ArgsType...>));
}

template <typename ClassType, typename... ArgsType>
Callable Callable::factoryShared(void)
{
    using SharedType = std::shared_ptr<ClassType>;
    return Callable(new Detail::CallableFunctionContainer<SharedType, SharedType, ArgsType...>(
        &std::make_shared<ClassType, ArgsType...>));
}

inline Callable::Callable(Detail::CallableContainer *impl):
    _impl(impl)
{
    return;
}

} //namespace Pothos
