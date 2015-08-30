// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "JavaProxy.hpp"
#include <iostream>
#include <cassert>
#include <cctype>

JavaProxyHandle::JavaProxyHandle(std::shared_ptr<JavaProxyEnvironment> env, jvalue value, char sig):
    env(env), value(value), sig(sig)
{
    assert(isupper(sig));
}

/***********************************************************************
 * helpers for dealing with primitives
 **********************************************************************/
char JavaProxyHandle::getSigFromClassType(jobject classType) const
{
    jclass cls = env->FindClass("java/util/Objects");
    jmethodID mid = env->GetStaticMethodID(cls, "equals", "(Ljava/lang/Object;Ljava/lang/Object;)Z");
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Boolean"))) return 'Z';
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Byte"))) return 'B';
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Character"))) return 'C';
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Short"))) return 'S';
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Integer"))) return 'I';
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Long"))) return 'J';
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Float"))) return 'F';
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Double"))) return 'D';
    if (env->env->CallStaticBooleanMethod(cls, mid, classType, env->getPrimitiveClassType("java/lang/Void"))) return 'V';
    return 'L';
}

jobject JavaProxyHandle::toJobject(void) const
{
    jclass cls = nullptr;
    jmethodID mid = nullptr;
    switch (this->sig)
    {
    case 'L': return value.l;
    case 'Z': cls = env->FindClass("java/lang/Boolean"); mid = env->GetMethodID(cls, "<init>", "(Z)V"); break;
    case 'B': cls = env->FindClass("java/lang/Byte"); mid = env->GetMethodID(cls, "<init>", "(B)V"); break;
    case 'C': cls = env->FindClass("java/lang/Char"); mid = env->GetMethodID(cls, "<init>", "(C)V"); break;
    case 'S': cls = env->FindClass("java/lang/Short"); mid = env->GetMethodID(cls, "<init>", "(S)V"); break;
    case 'I': cls = env->FindClass("java/lang/Integer"); mid = env->GetMethodID(cls, "<init>", "(I)V"); break;
    case 'J': cls = env->FindClass("java/lang/Long"); mid = env->GetMethodID(cls, "<init>", "(J)V"); break;
    case 'F': cls = env->FindClass("java/lang/Float"); mid = env->GetMethodID(cls, "<init>", "(F)V"); break;
    case 'D': cls = env->FindClass("java/lang/Double"); mid = env->GetMethodID(cls, "<init>", "(D)V"); break;
    }
    assert(cls != nullptr);
    assert(mid != nullptr);
    return env->env->NewObjectA(cls, mid, &value);
}

/***********************************************************************
 * string information
 **********************************************************************/
std::string JavaProxyHandle::toString(void) const
{
    jmethodID toString = env->GetMethodID("java/lang/Object", "toString", "()Ljava/lang/String;");
    return env->jstringToString(env->env->CallObjectMethod(this->toJobject(), toString));
}

std::string JavaProxyHandle::getClassName(void) const
{
    if (sig != 'L') return std::string(1, this->sig);
    if (value.l == nullptr) return "";
    jclass cls = env->env->GetObjectClass(value.l);
    assert(cls != nullptr);
    return env->getClassName(cls);
}

/***********************************************************************
 * Compare implementation
 **********************************************************************/
int JavaProxyHandle::compareTo(const Pothos::Proxy &proxy) const
{
    auto handle = env->getHandle(proxy);

    jclass Comparable = env->FindClass("java/lang/Comparable");
    jmethodID isInstance = env->GetMethodID("java/lang/Class", "isInstance", "(Ljava/lang/Object;)Z");
    jmethodID compareTo = env->GetMethodID(Comparable, "compareTo", "(Ljava/lang/Object;)I");
    jobject thisObj = this->toJobject();
    if (env->env->CallBooleanMethod(Comparable, isInstance, thisObj))
    {
        jint r = env->env->CallIntMethod(thisObj, compareTo, handle->toJobject());
        auto errorMsg = env->getErrorString();
        if (not errorMsg.empty())
        {
            throw Pothos::ProxyCompareError("JavaProxyHandle::compareTo()", errorMsg);
        }
        return r;
    }

    throw Pothos::ProxyCompareError("JavaProxyHandle::compareTo()", "not comparable");
}

/***********************************************************************
 * hash code implementation
 **********************************************************************/
size_t JavaProxyHandle::hashCode(void) const
{
    jclass Objects = env->FindClass("java/util/Objects");
    jmethodID hashCode = env->GetStaticMethodID(Objects, "hashCode", "(Ljava/lang/Object;)I");
    const int h = env->env->CallStaticIntMethod(Objects, hashCode, this->toJobject());
    auto errorMsg = env->getErrorString();
    if (not errorMsg.empty()) return size_t(this);
    return size_t(h);
}

/***********************************************************************
 * Call implementation
 **********************************************************************/
Pothos::Proxy JavaProxyHandle::call(const std::string &name, const Pothos::Proxy *args, const size_t numArgs)
{
    jint ret = env->jvm->AttachCurrentThread((void **)&env->env, nullptr);
    if (ret < 0) throw Pothos::ProxyHandleCallError("JavaProxyHandle::call("+name+")", "AttachCurrentThread failed");

    //Special reflective method to get the class type for a proxy handle.
    //This should not interfere with other methods since class is reserved.
    if (name == "class" and numArgs == 0)
    {
        return env->makeHandle(env->env->GetObjectClass(this->toJobject()));
    }

    //Cannot continue further, primitives have nothing to call.
    if (this->sig != 'L')
    {
        throw Pothos::ProxyHandleCallError("JavaProxyHandle::call("+name+")", "cannot call method on primitive");
    }

    /*******************************************************************
     * Step 0) handle field accessors and mutators
     ******************************************************************/
    const auto colon = name.find(":");
    if (colon != std::string::npos)
    {
        auto self = env->makeHandle(this->toJobject());
        if (name.substr(0, colon) == "set" and numArgs == 1)
        {
            return self.callProxy("class").callProxy("getField", name.substr(colon+1)).callProxy("set", self, args[0]);
        }
        else if (name.substr(0, colon) == "get" and numArgs == 0)
        {
            return self.callProxy("class").callProxy("getField", name.substr(colon+1)).callProxy("get", self);
        }
        else throw Pothos::ProxyHandleCallError(
            "PythonProxyHandle::call("+name+")", "unknown operation");
    }

    /*******************************************************************
     * Step 1) create vector of JavaProxyHandles from the args
     ******************************************************************/
    #define maxNumArgs 13
    if (numArgs > maxNumArgs) throw Pothos::ProxyHandleCallError("JavaProxyHandle::call("+name+")", "too many args");
    std::shared_ptr<JavaProxyHandle> argHandles[maxNumArgs];
    jvalue paramValues[maxNumArgs];
    char paramSig[maxNumArgs];
    for (size_t i = 0; i < numArgs; i++)
    {
        argHandles[i] = env->getHandle(args[i]);
        paramValues[i] = argHandles[i]->value;
    }

    /*******************************************************************
     * Step 2) locate the method to call
     ******************************************************************/
    const bool callConstructor = (name == "new");
    jmethodID isInstance = env->GetMethodID("java/lang/Class", "isInstance", "(Ljava/lang/Object;)Z");
    jmethodID isPrimitive = env->GetMethodID("java/lang/Class", "isPrimitive", "()Z");
    const bool isClass = env->env->CallBooleanMethod(env->FindClass("java/lang/Class"), isInstance, this->value.l);
    if (callConstructor and not isClass) throw Pothos::ProxyHandleCallError("JavaProxyHandle::call("+name+")", "cannot call constructor on instance");
    jclass reflectClass = env->FindClass(callConstructor?"java/lang/reflect/Constructor":"java/lang/reflect/Method");
    jclass modifierClass = env->FindClass("java/lang/reflect/Modifier");
    jmethodID isStatic = env->GetStaticMethodID(modifierClass, "isStatic", "(I)Z");
    jmethodID isPublic = env->GetStaticMethodID(modifierClass, "isPublic", "(I)Z");
    jmethodID getMethods = env->GetMethodID("java/lang/Class",
        callConstructor?"getConstructors":"getMethods",
        callConstructor?"()[Ljava/lang/reflect/Constructor;":"()[Ljava/lang/reflect/Method;");
    jobject classClass = callConstructor? value.l : env->env->GetObjectClass(value.l);
    jobjectArray methodObjs = (jobjectArray) env->env->CallObjectMethod(classClass, getMethods);
    jobject methodObjFound = nullptr;
    jboolean methodIsStatic;
    for (int i = 0; i < env->env->GetArrayLength(methodObjs); i++)
    {
        jobject methodObj = env->env->GetObjectArrayElement((jobjectArray)methodObjs, i);

        //check the modifiers for public and static status
        jint modifiers = env->env->CallIntMethod(methodObj, env->GetMethodID(reflectClass, "getModifiers", "()I"));
        if (not env->env->CallStaticBooleanMethod(modifierClass, isPublic, modifiers)) continue; //skip non-public methods

        //separate static and non-static based on call type
        methodIsStatic = env->env->CallStaticBooleanMethod(modifierClass, isStatic, modifiers);

        //check the name for a match
        if (not callConstructor)
        {
            jmethodID getName = env->GetMethodID(reflectClass, "getName", "()Ljava/lang/String;");
            std::string methodName = env->jstringToString(env->env->CallObjectMethod(methodObj, getName));
            if (methodName != name) continue;
        }

        //check the paramaters for a match
        jmethodID getParameterTypes = env->GetMethodID(reflectClass, "getParameterTypes", "()[Ljava/lang/Class;");
        jobjectArray params = (jobjectArray)env->env->CallObjectMethod(methodObj, getParameterTypes);
        if (env->env->GetArrayLength(params) != int(numArgs)) continue; //number of args must be the same as call
        for (int j = 0; j < env->env->GetArrayLength(params); j++)
        {
            jobject param = env->env->GetObjectArrayElement((jobjectArray)params, j);
            if (env->env->CallBooleanMethod(param, isInstance, argHandles[j]->toJobject()))
            {
                //can convert to object or its an instance
            }
            else if (argHandles[j]->sig != 'L' and env->env->CallBooleanMethod(param, isPrimitive))
            {
                //both are primitives -- conversion assumed possible
            }
            else goto loopAgain;
            paramSig[j] = this->getSigFromClassType(param);
        }

        //got here and params match
        methodObjFound = methodObj;
        break;

        loopAgain: continue;
    }
    if (methodObjFound == nullptr) throw Pothos::ProxyHandleCallError("JavaProxyHandle::call("+name+")", "cannot find method match");

    /*******************************************************************
     * Step 3) convert primative args to the right type
     ******************************************************************/
    for (size_t i = 0; i < numArgs; i++)
    {
        if (argHandles[i]->sig == paramSig[i]) continue;
        if (paramSig[i] == 'L') //convert to the object
        {
            paramValues[i].l = argHandles[i]->toJobject();
            continue;
        }
        //else its a primitive conversion

        jvalue &value = paramValues[i];
        double dVal = 0.0; long long lVal = 0;

        //convert input to intermediate
        switch (argHandles[i]->sig)
        {
        #define doInputCase(Sig, Field) case Sig: {dVal = static_cast<double>(value.Field); lVal = static_cast<long long>(value.Field); break;}
        doInputCase('Z', z)
        doInputCase('B', b)
        doInputCase('C', c)
        doInputCase('S', s)
        doInputCase('I', i)
        doInputCase('J', j)
        doInputCase('F', f)
        doInputCase('D', d)
        default: assert(false);
        }

        //convert intermediate to output
        switch (paramSig[i])
        {
        #define doOutputCaseL(Sig, Field, T) case Sig: {value.Field = static_cast<T>(lVal); break;}
        #define doOutputCaseD(Sig, Field, T) case Sig: {value.Field = static_cast<T>(dVal); break;}
        doOutputCaseL('Z', z, jboolean)
        doOutputCaseL('B', b, jbyte)
        doOutputCaseL('C', c, jchar)
        doOutputCaseL('S', s, jshort)
        doOutputCaseL('I', i, jint)
        doOutputCaseL('J', j, jlong)
        doOutputCaseD('F', f, jfloat)
        doOutputCaseD('D', d, jdouble)
        default: assert(false);
        }
    }

    /*******************************************************************
     * Step 4) get the return type so we know what to call
     ******************************************************************/
    char returnSig = 'L';
    if (not callConstructor)
    {
        jmethodID getReturnType = env->GetMethodID(reflectClass, "getReturnType", "()Ljava/lang/Class;");
        jobject returnType = env->env->CallObjectMethod(methodObjFound, getReturnType);
        returnSig = this->getSigFromClassType(returnType);
    }

    /*******************************************************************
     * Step 5) call the method
     ******************************************************************/
    jmethodID method = env->env->FromReflectedMethod(methodObjFound);
    jvalue result;
    if (callConstructor)
    {
        result.l = env->env->NewObjectA((jclass)(value.l), method, paramValues);
    }
    else if (methodIsStatic)
    {
        result = env->CallStaticMethodA(returnSig, (jclass)(value.l), method, paramValues);
    }
    else
    {
        result = env->CallMethodA(returnSig, value.l, method, paramValues);
    }

    /*******************************************************************
     * Step 6) deal with exceptions
     ******************************************************************/
    auto errorMsg = env->getErrorString();
    if (not errorMsg.empty())
    {
        throw Pothos::ProxyExceptionMessage(errorMsg);
    }

    /*******************************************************************
     * Step 7) cast the return result
     ******************************************************************/
    if (returnSig == 'V') return Pothos::Proxy(); //void return special case
    return env->makeHandle(result, returnSig);
}
