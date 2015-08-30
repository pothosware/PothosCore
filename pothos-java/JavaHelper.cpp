// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include "JavaProxy.hpp"
#include <cassert>

/***********************************************************************
 * helpers that check the result -- debugging is easier
 **********************************************************************/
std::string JavaProxyEnvironment::getErrorString(void)
{
    jthrowable thrown = env->ExceptionOccurred();
    if (thrown == nullptr) return "";
    jmethodID getMessage = this->GetMethodID("java/lang/Throwable", "getMessage", "()Ljava/lang/String;");
    jobject message = env->CallObjectMethod(thrown, getMessage);
    env->ExceptionClear();
    return this->jstringToString(message);
}

jclass JavaProxyEnvironment::FindClass(const char *name)
{
    jclass cls = env->FindClass(name);
    if (cls == nullptr)
    {
        throw Pothos::Exception(std::string()+"Java::FindClass("+name+")");
    }
    return cls;
}

jobject JavaProxyEnvironment::forName(const char *name)
{
    jclass cls = this->FindClass("java/lang/Class");
    jmethodID forName = this->GetStaticMethodID(cls, "forName", "(Ljava/lang/String;)Ljava/lang/Class;");
    jobject classType = env->CallStaticObjectMethod(cls, forName, env->NewStringUTF(name));
    return classType;
}

jobject JavaProxyEnvironment::getPrimitiveClassType(const char *name)
{
    jclass typeClass = this->FindClass(name);
    jfieldID getType = env->GetStaticFieldID(typeClass, "TYPE", "Ljava/lang/Class;");
    jobject classType = env->GetStaticObjectField(typeClass, getType);
    assert(classType != nullptr);
    return classType;
}

jmethodID JavaProxyEnvironment::GetMethodID(jclass cls, const char *name, const char *sig)
{
    jmethodID id = env->GetMethodID(cls, name, sig);
    if (id == nullptr)
    {
        throw Pothos::Exception(std::string()+"Java::GetMethodID(cls, "+name+", "+sig+")");
    }
    return id;
}

jmethodID JavaProxyEnvironment::GetStaticMethodID(jclass cls, const char *name, const char *sig)
{
    jmethodID id = env->GetStaticMethodID(cls, name, sig);
    if (id == nullptr)
    {
        throw Pothos::Exception(std::string()+"Java::GetStaticMethodID(cls, "+name+", "+sig+")");
    }
    return id;
}

jmethodID JavaProxyEnvironment::GetMethodID(const char *cls, const char *name, const char *sig)
{
    return this->GetMethodID(this->FindClass(cls), name, sig);
}

jmethodID JavaProxyEnvironment::GetStaticMethodID(const char *cls, const char *name, const char *sig)
{
    return this->GetStaticMethodID(this->FindClass(cls), name, sig);
}

std::string JavaProxyEnvironment::jstringToString(jobject str)
{
    const char *p = env->GetStringUTFChars((jstring)str, nullptr);
    const std::string s(p, env->GetStringUTFLength((jstring)str));
    env->ReleaseStringUTFChars((jstring)str, p);
    return s;
}

std::string JavaProxyEnvironment::getClassName(jobject cls)
{
    jmethodID getName = this->GetMethodID("java/lang/Class", "getName", "()Ljava/lang/String;");
    jobject str = this->env->CallObjectMethod(cls, getName);
    return this->jstringToString(str);
}

/***********************************************************************
 * helpers that dispatch based on return type
 **********************************************************************/
jvalue JavaProxyEnvironment::CallMethodA(const char retType, jobject obj, jmethodID method, jvalue *args)
{
    jvalue result = {};
    switch (retType)
    {
    case 'L': result.l = env->CallObjectMethodA(obj, method, args); break;
    case 'Z': result.z = env->CallBooleanMethodA(obj, method, args); break;
    case 'B': result.b = env->CallByteMethodA(obj, method, args); break;
    case 'C': result.c = env->CallCharMethodA(obj, method, args); break;
    case 'S': result.s = env->CallShortMethodA(obj, method, args); break;
    case 'I': result.i = env->CallIntMethodA(obj, method, args); break;
    case 'J': result.j = env->CallLongMethodA(obj, method, args); break;
    case 'F': result.f = env->CallFloatMethodA(obj, method, args); break;
    case 'D': result.d = env->CallDoubleMethodA(obj, method, args); break;
    case 'V': env->CallVoidMethodA(obj, method, args); break;
    default: throw Pothos::Exception("JavaProxyEnvironment::CallMethodA("+std::string(1, retType)+")", "unknown return type");
    }
    return result;
}

jvalue JavaProxyEnvironment::CallStaticMethodA(const char retType, jclass cls, jmethodID method, jvalue *args)
{
    jvalue result = {};
    switch (retType)
    {
    case 'L': result.l = env->CallStaticObjectMethodA(cls, method, args); break;
    case 'Z': result.z = env->CallStaticBooleanMethodA(cls, method, args); break;
    case 'B': result.b = env->CallStaticByteMethodA(cls, method, args); break;
    case 'C': result.c = env->CallStaticCharMethodA(cls, method, args); break;
    case 'S': result.s = env->CallStaticShortMethodA(cls, method, args); break;
    case 'I': result.i = env->CallStaticIntMethodA(cls, method, args); break;
    case 'J': result.j = env->CallStaticLongMethodA(cls, method, args); break;
    case 'F': result.f = env->CallStaticFloatMethodA(cls, method, args); break;
    case 'D': result.d = env->CallStaticDoubleMethodA(cls, method, args); break;
    case 'V': env->CallStaticVoidMethodA(cls, method, args); break;
    default: throw Pothos::Exception("JavaProxyEnvironment::CallStaticMethodA("+std::string(1, retType)+")", "unknown return type");
    }
    return result;
}
