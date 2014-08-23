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
    return this->bind(Object::make(std::forward<ValueType>(val)), argNo);
}

namespace Detail {
/***********************************************************************
 * Function specialization for return type with 0 args
 **********************************************************************/
template <typename ReturnType>
struct CallableFunctionContainer0 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer0(const FcnType &fcn):
        _fcn(std::bind(fcn))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 0;
    }

    const std::type_info &type(const int /*argNo*/)
    {
        return typeid(ReturnType);
    }

    Object call(const Object * /*args*/)
    {
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn)
        {
            return Object::make(fcn());
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn)
        {
            fcn(); return Object();
        }
    };

    std::function<ReturnType()> _fcn;
};

template <typename ClassType>
ClassType CallableFactoryWrapper()
{
    return ClassType();
}

template <typename ClassType>
ClassType *CallableFactoryNewWrapper()
{
    return new ClassType();
}

template <typename ClassType>
std::shared_ptr<ClassType> CallableFactorySharedWrapper()
{
    return std::shared_ptr<ClassType>(new ClassType());
}
/***********************************************************************
 * Function specialization for return type with 1 args
 **********************************************************************/
template <typename ReturnType, typename A0>
struct CallableFunctionContainer1 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer1(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 1;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0)
        {
            return Object::make(fcn(a0));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0)
        {
            fcn(a0); return Object();
        }
    };

    std::function<ReturnType(A0)> _fcn;
};

template <typename ClassType, typename A0>
ClassType CallableFactoryWrapper(const A0 &a0)
{
    return ClassType(a0);
}

template <typename ClassType, typename A0>
ClassType *CallableFactoryNewWrapper(const A0 &a0)
{
    return new ClassType(a0);
}

template <typename ClassType, typename A0>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0)
{
    return std::shared_ptr<ClassType>(new ClassType(a0));
}
/***********************************************************************
 * Function specialization for return type with 2 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1>
struct CallableFunctionContainer2 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer2(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 2;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1)
        {
            return Object::make(fcn(a0, a1));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1)
        {
            fcn(a0, a1); return Object();
        }
    };

    std::function<ReturnType(A0, A1)> _fcn;
};

template <typename ClassType, typename A0, typename A1>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1)
{
    return ClassType(a0, a1);
}

template <typename ClassType, typename A0, typename A1>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1)
{
    return new ClassType(a0, a1);
}

template <typename ClassType, typename A0, typename A1>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1));
}
/***********************************************************************
 * Function specialization for return type with 3 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2>
struct CallableFunctionContainer3 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer3(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 3;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        if (argNo == 2) return typeid(A2);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        auto &a2 = args[2].extract<A2>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1, a2);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2)
        {
            return Object::make(fcn(a0, a1, a2));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2)
        {
            fcn(a0, a1, a2); return Object();
        }
    };

    std::function<ReturnType(A0, A1, A2)> _fcn;
};

template <typename ClassType, typename A0, typename A1, typename A2>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1, const A2 &a2)
{
    return ClassType(a0, a1, a2);
}

template <typename ClassType, typename A0, typename A1, typename A2>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1, const A2 &a2)
{
    return new ClassType(a0, a1, a2);
}

template <typename ClassType, typename A0, typename A1, typename A2>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1, const A2 &a2)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1, a2));
}
/***********************************************************************
 * Function specialization for return type with 4 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3>
struct CallableFunctionContainer4 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer4(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 4;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        if (argNo == 2) return typeid(A2);
        if (argNo == 3) return typeid(A3);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        auto &a2 = args[2].extract<A2>();
        auto &a3 = args[3].extract<A3>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1, a2, a3);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3)
        {
            return Object::make(fcn(a0, a1, a2, a3));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3)
        {
            fcn(a0, a1, a2, a3); return Object();
        }
    };

    std::function<ReturnType(A0, A1, A2, A3)> _fcn;
};

template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ClassType(a0, a1, a2, a3);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return new ClassType(a0, a1, a2, a3);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1, a2, a3));
}
/***********************************************************************
 * Function specialization for return type with 5 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4>
struct CallableFunctionContainer5 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer5(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 5;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        if (argNo == 2) return typeid(A2);
        if (argNo == 3) return typeid(A3);
        if (argNo == 4) return typeid(A4);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        auto &a2 = args[2].extract<A2>();
        auto &a3 = args[3].extract<A3>();
        auto &a4 = args[4].extract<A4>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1, a2, a3, a4);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
        {
            return Object::make(fcn(a0, a1, a2, a3, a4));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
        {
            fcn(a0, a1, a2, a3, a4); return Object();
        }
    };

    std::function<ReturnType(A0, A1, A2, A3, A4)> _fcn;
};

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ClassType(a0, a1, a2, a3, a4);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return new ClassType(a0, a1, a2, a3, a4);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1, a2, a3, a4));
}
/***********************************************************************
 * Function specialization for return type with 6 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
struct CallableFunctionContainer6 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer6(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 6;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        if (argNo == 2) return typeid(A2);
        if (argNo == 3) return typeid(A3);
        if (argNo == 4) return typeid(A4);
        if (argNo == 5) return typeid(A5);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        auto &a2 = args[2].extract<A2>();
        auto &a3 = args[3].extract<A3>();
        auto &a4 = args[4].extract<A4>();
        auto &a5 = args[5].extract<A5>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1, a2, a3, a4, a5);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
        {
            return Object::make(fcn(a0, a1, a2, a3, a4, a5));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
        {
            fcn(a0, a1, a2, a3, a4, a5); return Object();
        }
    };

    std::function<ReturnType(A0, A1, A2, A3, A4, A5)> _fcn;
};

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ClassType(a0, a1, a2, a3, a4, a5);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return new ClassType(a0, a1, a2, a3, a4, a5);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1, a2, a3, a4, a5));
}
/***********************************************************************
 * Function specialization for return type with 7 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
struct CallableFunctionContainer7 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer7(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 7;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        if (argNo == 2) return typeid(A2);
        if (argNo == 3) return typeid(A3);
        if (argNo == 4) return typeid(A4);
        if (argNo == 5) return typeid(A5);
        if (argNo == 6) return typeid(A6);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        auto &a2 = args[2].extract<A2>();
        auto &a3 = args[3].extract<A3>();
        auto &a4 = args[4].extract<A4>();
        auto &a5 = args[5].extract<A5>();
        auto &a6 = args[6].extract<A6>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1, a2, a3, a4, a5, a6);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
        {
            return Object::make(fcn(a0, a1, a2, a3, a4, a5, a6));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
        {
            fcn(a0, a1, a2, a3, a4, a5, a6); return Object();
        }
    };

    std::function<ReturnType(A0, A1, A2, A3, A4, A5, A6)> _fcn;
};

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return ClassType(a0, a1, a2, a3, a4, a5, a6);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return new ClassType(a0, a1, a2, a3, a4, a5, a6);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1, a2, a3, a4, a5, a6));
}
/***********************************************************************
 * Function specialization for return type with 8 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
struct CallableFunctionContainer8 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer8(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 8;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        if (argNo == 2) return typeid(A2);
        if (argNo == 3) return typeid(A3);
        if (argNo == 4) return typeid(A4);
        if (argNo == 5) return typeid(A5);
        if (argNo == 6) return typeid(A6);
        if (argNo == 7) return typeid(A7);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        auto &a2 = args[2].extract<A2>();
        auto &a3 = args[3].extract<A3>();
        auto &a4 = args[4].extract<A4>();
        auto &a5 = args[5].extract<A5>();
        auto &a6 = args[6].extract<A6>();
        auto &a7 = args[7].extract<A7>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1, a2, a3, a4, a5, a6, a7);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
        {
            return Object::make(fcn(a0, a1, a2, a3, a4, a5, a6, a7));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
        {
            fcn(a0, a1, a2, a3, a4, a5, a6, a7); return Object();
        }
    };

    std::function<ReturnType(A0, A1, A2, A3, A4, A5, A6, A7)> _fcn;
};

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    return ClassType(a0, a1, a2, a3, a4, a5, a6, a7);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    return new ClassType(a0, a1, a2, a3, a4, a5, a6, a7);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1, a2, a3, a4, a5, a6, a7));
}
/***********************************************************************
 * Function specialization for return type with 9 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
struct CallableFunctionContainer9 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer9(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 9;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        if (argNo == 2) return typeid(A2);
        if (argNo == 3) return typeid(A3);
        if (argNo == 4) return typeid(A4);
        if (argNo == 5) return typeid(A5);
        if (argNo == 6) return typeid(A6);
        if (argNo == 7) return typeid(A7);
        if (argNo == 8) return typeid(A8);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        auto &a2 = args[2].extract<A2>();
        auto &a3 = args[3].extract<A3>();
        auto &a4 = args[4].extract<A4>();
        auto &a5 = args[5].extract<A5>();
        auto &a6 = args[6].extract<A6>();
        auto &a7 = args[7].extract<A7>();
        auto &a8 = args[8].extract<A8>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1, a2, a3, a4, a5, a6, a7, a8);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
        {
            return Object::make(fcn(a0, a1, a2, a3, a4, a5, a6, a7, a8));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
        {
            fcn(a0, a1, a2, a3, a4, a5, a6, a7, a8); return Object();
        }
    };

    std::function<ReturnType(A0, A1, A2, A3, A4, A5, A6, A7, A8)> _fcn;
};

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    return ClassType(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    return new ClassType(a0, a1, a2, a3, a4, a5, a6, a7, a8);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1, a2, a3, a4, a5, a6, a7, a8));
}
/***********************************************************************
 * Function specialization for return type with 10 args
 **********************************************************************/
template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
struct CallableFunctionContainer10 : Detail::CallableContainer
{
    template <typename FcnType>
    CallableFunctionContainer10(const FcnType &fcn):
        _fcn(std::bind(fcn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7, std::placeholders::_8, std::placeholders::_9, std::placeholders::_10))
    {
        return;
    }

    size_t getNumArgs(void) const
    {
        return 10;
    }

    const std::type_info &type(const int argNo)
    {
        if (argNo == 0) return typeid(A0);
        if (argNo == 1) return typeid(A1);
        if (argNo == 2) return typeid(A2);
        if (argNo == 3) return typeid(A3);
        if (argNo == 4) return typeid(A4);
        if (argNo == 5) return typeid(A5);
        if (argNo == 6) return typeid(A6);
        if (argNo == 7) return typeid(A7);
        if (argNo == 8) return typeid(A8);
        if (argNo == 9) return typeid(A9);
        return typeid(ReturnType);
    }

    Object call(const Object * args)
    {
        auto &a0 = args[0].extract<A0>();
        auto &a1 = args[1].extract<A1>();
        auto &a2 = args[2].extract<A2>();
        auto &a3 = args[3].extract<A3>();
        auto &a4 = args[4].extract<A4>();
        auto &a5 = args[5].extract<A5>();
        auto &a6 = args[6].extract<A6>();
        auto &a7 = args[7].extract<A7>();
        auto &a8 = args[8].extract<A8>();
        auto &a9 = args[9].extract<A9>();
        return CallHelper<
            decltype(_fcn), std::is_void<ReturnType>::value
        >::call(_fcn, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

    //! templated call of function for optional return type
    template <typename FcnType, bool Condition> struct CallHelper;
    template <typename FcnType> struct CallHelper<FcnType, false>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
        {
            return Object::make(fcn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9));
        }
    };
    template <typename FcnType> struct CallHelper<FcnType, true>
    {
        static Object call(const FcnType &fcn, const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
        {
            fcn(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9); return Object();
        }
    };

    std::function<ReturnType(A0, A1, A2, A3, A4, A5, A6, A7, A8, A9)> _fcn;
};

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
ClassType CallableFactoryWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
    return ClassType(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
ClassType *CallableFactoryNewWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
    return new ClassType(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
std::shared_ptr<ClassType> CallableFactorySharedWrapper(const A0 &a0, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
    return std::shared_ptr<ClassType>(new ClassType(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9));
}

} //namespace Detail

/***********************************************************************
 * Templated factory/constructor calls with 0 args
 **********************************************************************/
template <typename ReturnType, typename ClassType>
Callable::Callable(ReturnType(ClassType::*fcn)()):
    _impl(new Detail::CallableFunctionContainer1<ReturnType, ClassType &>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType>
Callable::Callable(ReturnType(ClassType::*fcn)() const):
    _impl(new Detail::CallableFunctionContainer1<ReturnType, const ClassType &>(fcn))
{
    return;
}

template <typename ReturnType>
Callable::Callable(ReturnType(*fcn)()):
    _impl(new Detail::CallableFunctionContainer0<ReturnType>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)())
{
    return Callable(fcn);
}

template <typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)() const)
{
    return Callable(fcn);
}

template <typename ReturnType>
Callable Callable::make(ReturnType(*fcn)())
{
    return Callable(fcn);
}

template <typename ClassType>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType>);
}

template <typename ClassType>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType>);
}

template <typename ClassType>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType>);
}

/***********************************************************************
 * Templated factory/constructor calls with 1 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0>
Callable::Callable(ReturnType(ClassType::*fcn)(A0)):
    _impl(new Detail::CallableFunctionContainer2<ReturnType, ClassType &, A0>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0>
Callable::Callable(ReturnType(ClassType::*fcn)(A0) const):
    _impl(new Detail::CallableFunctionContainer2<ReturnType, const ClassType &, A0>(fcn))
{
    return;
}

template <typename ReturnType, typename A0>
Callable::Callable(ReturnType(*fcn)(A0)):
    _impl(new Detail::CallableFunctionContainer1<ReturnType, A0>(fcn))
{
    return;
}

template <typename A0, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0))
{
    return Callable(fcn);
}

template <typename A0, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0) const)
{
    return Callable(fcn);
}

template <typename A0, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0>);
}

template <typename ClassType, typename A0>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0>);
}

template <typename ClassType, typename A0>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0>);
}

/***********************************************************************
 * Templated factory/constructor calls with 2 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0, typename A1>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1)):
    _impl(new Detail::CallableFunctionContainer3<ReturnType, ClassType &, A0, A1>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0, typename A1>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1) const):
    _impl(new Detail::CallableFunctionContainer3<ReturnType, const ClassType &, A0, A1>(fcn))
{
    return;
}

template <typename ReturnType, typename A0, typename A1>
Callable::Callable(ReturnType(*fcn)(A0, A1)):
    _impl(new Detail::CallableFunctionContainer2<ReturnType, A0, A1>(fcn))
{
    return;
}

template <typename A0, typename A1, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1))
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1) const)
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0, A1))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0, typename A1>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0, A1>);
}

template <typename ClassType, typename A0, typename A1>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0, A1>);
}

template <typename ClassType, typename A0, typename A1>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0, A1>);
}

/***********************************************************************
 * Templated factory/constructor calls with 3 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2)):
    _impl(new Detail::CallableFunctionContainer4<ReturnType, ClassType &, A0, A1, A2>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2) const):
    _impl(new Detail::CallableFunctionContainer4<ReturnType, const ClassType &, A0, A1, A2>(fcn))
{
    return;
}

template <typename ReturnType, typename A0, typename A1, typename A2>
Callable::Callable(ReturnType(*fcn)(A0, A1, A2)):
    _impl(new Detail::CallableFunctionContainer3<ReturnType, A0, A1, A2>(fcn))
{
    return;
}

template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2))
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2) const)
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0, A1, A2))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0, typename A1, typename A2>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0, A1, A2>);
}

template <typename ClassType, typename A0, typename A1, typename A2>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0, A1, A2>);
}

template <typename ClassType, typename A0, typename A1, typename A2>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0, A1, A2>);
}

/***********************************************************************
 * Templated factory/constructor calls with 4 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3)):
    _impl(new Detail::CallableFunctionContainer5<ReturnType, ClassType &, A0, A1, A2, A3>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3) const):
    _impl(new Detail::CallableFunctionContainer5<ReturnType, const ClassType &, A0, A1, A2, A3>(fcn))
{
    return;
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3>
Callable::Callable(ReturnType(*fcn)(A0, A1, A2, A3)):
    _impl(new Detail::CallableFunctionContainer4<ReturnType, A0, A1, A2, A3>(fcn))
{
    return;
}

template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3))
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3) const)
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0, A1, A2, A3))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0, A1, A2, A3>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0, A1, A2, A3>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0, A1, A2, A3>);
}

/***********************************************************************
 * Templated factory/constructor calls with 5 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4)):
    _impl(new Detail::CallableFunctionContainer6<ReturnType, ClassType &, A0, A1, A2, A3, A4>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4) const):
    _impl(new Detail::CallableFunctionContainer6<ReturnType, const ClassType &, A0, A1, A2, A3, A4>(fcn))
{
    return;
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4>
Callable::Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4)):
    _impl(new Detail::CallableFunctionContainer5<ReturnType, A0, A1, A2, A3, A4>(fcn))
{
    return;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4))
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4) const)
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0, A1, A2, A3, A4))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0, A1, A2, A3, A4>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0, A1, A2, A3, A4>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0, A1, A2, A3, A4>);
}

/***********************************************************************
 * Templated factory/constructor calls with 6 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5)):
    _impl(new Detail::CallableFunctionContainer7<ReturnType, ClassType &, A0, A1, A2, A3, A4, A5>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5) const):
    _impl(new Detail::CallableFunctionContainer7<ReturnType, const ClassType &, A0, A1, A2, A3, A4, A5>(fcn))
{
    return;
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
Callable::Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5)):
    _impl(new Detail::CallableFunctionContainer6<ReturnType, A0, A1, A2, A3, A4, A5>(fcn))
{
    return;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5))
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5) const)
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0, A1, A2, A3, A4, A5>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0, A1, A2, A3, A4, A5>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0, A1, A2, A3, A4, A5>);
}

/***********************************************************************
 * Templated factory/constructor calls with 7 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6)):
    _impl(new Detail::CallableFunctionContainer8<ReturnType, ClassType &, A0, A1, A2, A3, A4, A5, A6>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6) const):
    _impl(new Detail::CallableFunctionContainer8<ReturnType, const ClassType &, A0, A1, A2, A3, A4, A5, A6>(fcn))
{
    return;
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Callable::Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6)):
    _impl(new Detail::CallableFunctionContainer7<ReturnType, A0, A1, A2, A3, A4, A5, A6>(fcn))
{
    return;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6))
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6) const)
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6>);
}

/***********************************************************************
 * Templated factory/constructor calls with 8 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7)):
    _impl(new Detail::CallableFunctionContainer9<ReturnType, ClassType &, A0, A1, A2, A3, A4, A5, A6, A7>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7) const):
    _impl(new Detail::CallableFunctionContainer9<ReturnType, const ClassType &, A0, A1, A2, A3, A4, A5, A6, A7>(fcn))
{
    return;
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Callable::Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6, A7)):
    _impl(new Detail::CallableFunctionContainer8<ReturnType, A0, A1, A2, A3, A4, A5, A6, A7>(fcn))
{
    return;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7))
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7) const)
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6, A7))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6, A7>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6, A7>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6, A7>);
}

/***********************************************************************
 * Templated factory/constructor calls with 9 args
 **********************************************************************/
template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8)):
    _impl(new Detail::CallableFunctionContainer10<ReturnType, ClassType &, A0, A1, A2, A3, A4, A5, A6, A7, A8>(fcn))
{
    return;
}

template <typename ReturnType, typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Callable::Callable(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const):
    _impl(new Detail::CallableFunctionContainer10<ReturnType, const ClassType &, A0, A1, A2, A3, A4, A5, A6, A7, A8>(fcn))
{
    return;
}

template <typename ReturnType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Callable::Callable(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8)):
    _impl(new Detail::CallableFunctionContainer9<ReturnType, A0, A1, A2, A3, A4, A5, A6, A7, A8>(fcn))
{
    return;
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8))
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType, typename ClassType>
Callable Callable::make(ReturnType(ClassType::*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8) const)
{
    return Callable(fcn);
}

template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename ReturnType>
Callable Callable::make(ReturnType(*fcn)(A0, A1, A2, A3, A4, A5, A6, A7, A8))
{
    return Callable(fcn);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Callable Callable::factory(void)
{
    return Callable(&Detail::CallableFactoryWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6, A7, A8>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Callable Callable::factoryNew(void)
{
    return Callable(&Detail::CallableFactoryNewWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6, A7, A8>);
}

template <typename ClassType, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
Callable Callable::factoryShared(void)
{
    return Callable(&Detail::CallableFactorySharedWrapper<ClassType, A0, A1, A2, A3, A4, A5, A6, A7, A8>);
}

} //namespace Pothos
