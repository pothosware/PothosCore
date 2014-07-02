// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "ManagedProxy.hpp"
#include <Pothos/Managed.hpp>
#include <Pothos/Object.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/Format.h>
#include <cassert>
#include <iostream>

ManagedProxyHandle::ManagedProxyHandle(std::shared_ptr<ManagedProxyEnvironment> env, const Pothos::Object &obj):
    env(env), obj(obj)
{
    return;
}

int ManagedProxyHandle::compareTo(const Pothos::Proxy &proxy) const
{
    try
    {
        return this->obj.compareTo(env->getHandle(proxy)->obj);
    }
    catch(const Pothos::Exception &ex)
    {
        throw Pothos::ProxyCompareError("ManagedProxyHandle::compareTo()", ex);
    }
}

Pothos::Proxy ManagedProxyHandle::call(const std::string &name, const Pothos::Proxy *args, const size_t numArgs)
{
    const bool isManagedClass = obj.type() == typeid(Pothos::ManagedClass);
    const bool callConstructor = isManagedClass and name == "new";
    const bool callStaticMethod = isManagedClass and not callConstructor;
    const bool callMethod = not isManagedClass;

    /*******************************************************************
     * Step 1) locate the managed class for the held object
     ******************************************************************/
    Pothos::ManagedClass cls;
    if (isManagedClass) cls = obj.extract<Pothos::ManagedClass>();
    else
    {
        try
        {
            cls = Pothos::ManagedClass::lookup(obj.type());
        }
        catch(const Pothos::ManagedClassLookupError &)
        {
            //its ok, cls will be empty
        }
    }

    /*******************************************************************
     * Step 2) extract the list of calls
     ******************************************************************/
    std::vector<Pothos::Callable> calls;
    Pothos::Callable opaqueCall;
    Pothos::Callable wildcardCall;

    if (callConstructor)
    {
        calls = cls.getConstructors();
        opaqueCall = cls.getOpaqueConstructor();
    }
    else if (callStaticMethod)
    {
        try {calls = cls.getStaticMethods(name);}
        catch (const Pothos::ManagedClassNameError &){}
        try {opaqueCall = cls.getOpaqueStaticMethod(name);}
        catch (const Pothos::ManagedClassNameError &){}
        wildcardCall = cls.getWildcardStaticMethod();
    }
    else if (callMethod)
    {
        try {calls = cls.getMethods(name);}
        catch (const Pothos::ManagedClassNameError &){}
        try {opaqueCall = cls.getOpaqueMethod(name);}
        catch (const Pothos::ManagedClassNameError &){}
        wildcardCall = cls.getWildcardMethod();
    }

    /*******************************************************************
     * Step 3) create an argument list
     ******************************************************************/
    std::vector<Pothos::Object> argObjs;

    //class method, insert this handle as an instance
    if (callMethod)
    {
        if (this->obj.type() == cls.type())
        {
            argObjs.push_back(this->obj);
        }
        else if (this->obj.type() == cls.sharedType())
        {
            const auto &toWrapper = cls.getSharedToWrapper();
            argObjs.push_back(toWrapper.opaqueCall(&(this->obj), 1));
        }
        else if (this->obj.type() == cls.pointerType())
        {
            const auto &toWrapper = cls.getPointerToWrapper();
            argObjs.push_back(toWrapper.opaqueCall(&(this->obj), 1));
        }
        assert(not argObjs.empty());
    }

    //iterate through the other objects in args
    for (size_t i = 0; i < numArgs; i++)
    {
        try
        {
            argObjs.push_back(env->getHandle(args[i])->obj);
        }
        catch(const Pothos::Exception &ex)
        {
            throw Pothos::ProxyHandleCallError(
                "ManagedProxyHandle::call("+name+")",
                Poco::format("convert arg %d - %s", int(i), ex.displayText()));
        }
    }

    if (callMethod) assert(not argObjs.empty());

    /*******************************************************************
     * Step 3) find the best match for the call
     ******************************************************************/
    Pothos::Callable call;
    bool doOpaqueCall = false;
    bool doWildcardCall = false;
    for (const auto &c : calls)
    {
        if (c.getNumArgs() != argObjs.size()) goto failMatch;
        for (size_t a = 0; a < c.getNumArgs(); a++)
        {
            if (not argObjs[a].canConvert(c.type(a))) goto failMatch;
        }
        call = c;
        failMatch: continue;
    }
    if (not call and opaqueCall)
    {
        doOpaqueCall = true;
        call = opaqueCall;
    }
    if (not call and wildcardCall)
    {
        doWildcardCall = true;
        call = wildcardCall;
    }

    //attempt to make the call on a base class
    //always try to call the base class first if there is a wildcard handler
    if (callMethod and (not call or wildcardCall))
    {
        for (const auto &toBase : cls.getBaseClassConverters())
        {
            try
            {
                return env->makeHandle(toBase.opaqueCall(&argObjs.at(0), 1)).getHandle()->call(name, args, numArgs);
            }
            catch (const Pothos::ProxyHandleCallError &) {}
        }
    }

    //searching base classes failed, so we can error out this way if calls are empty
    if (calls.empty() and not opaqueCall and not wildcardCall)
    {
        throw Pothos::ProxyHandleCallError("ManagedProxyHandle::call("+name+")", "no available calls :" + obj.toString());
    }

    //otherwise just assume there was no possible match for the given args
    if (not call) throw Pothos::ProxyHandleCallError("ManagedProxyHandle::call("+name+")", "method match failed");

    /*******************************************************************
     * Step 4) make the call
     ******************************************************************/
    Pothos::Object result;
    try
    {
        Pothos::Object oArgs[4]; //yes this is so META
        size_t oArgsIndex = 0;
        if (doOpaqueCall or doWildcardCall)
        {
            if (callMethod) oArgs[oArgsIndex++] = argObjs.front();
            if (doWildcardCall) oArgs[oArgsIndex++] = Pothos::Object(name);
            if (callMethod)
            {
                assert(argObjs.size() > 0);
                oArgs[oArgsIndex++] = Pothos::Object(reinterpret_cast<const Pothos::Object *>(argObjs.data()+1));
                oArgs[oArgsIndex++] = Pothos::Object(size_t(argObjs.size()-1));
            }
            else
            {
                oArgs[oArgsIndex++] = Pothos::Object(reinterpret_cast<const Pothos::Object *>(argObjs.data()));
                oArgs[oArgsIndex++] = Pothos::Object(size_t(argObjs.size()));
            }
            result = call.opaqueCall(oArgs, oArgsIndex);
            if (not callConstructor)
            {
                assert(result.type() == typeid(Pothos::Object));
                Pothos::Object container = result; //tmp container since we are reassigning and extracting from result
                result = container.extract<Pothos::Object>();
            }
        }
        else result = call.opaqueCall(argObjs.data(), argObjs.size());
    }
    catch(const Pothos::Exception &ex)
    {
        throw Pothos::ProxyExceptionMessage(ex.displayText());
    }
    catch(const std::exception &ex)
    {
        throw Pothos::ProxyExceptionMessage(ex.what());
    }
    catch(...)
    {
        throw Pothos::ProxyExceptionMessage("unknown");
    }

    if (result.type() == typeid(Pothos::Proxy)) return result.extract<Pothos::Proxy>();
    return env->makeHandle(result);
}

std::string ManagedProxyHandle::toString(void) const
{
    return this->obj.toString();
}

std::string ManagedProxyHandle::getClassName(void) const
{
    return Pothos::Util::typeInfoToString(this->obj.type());
}
