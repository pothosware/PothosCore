// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Proxy.hpp>
#include <Pothos/Callable.hpp>
#include <jni.h>

class JavaProxyHandle;

/***********************************************************************
 * custom java environment overload
 **********************************************************************/
class JavaProxyEnvironment :
    public Pothos::ProxyEnvironment
{
public:
    JavaProxyEnvironment(const Pothos::ProxyEnvironmentArgs &);

    Pothos::Proxy makeHandle(jvalue value, char sig);
    Pothos::Proxy makeHandle(jobject obj)
    {
        jvalue value; value.l = obj;
        return this->makeHandle(value, 'L');
    }

    std::shared_ptr<JavaProxyHandle> getHandle(const Pothos::Proxy &proxy);

    std::string getName(void) const
    {
        return "java";
    }

    Pothos::Proxy findProxy(const std::string &name);

    std::string getErrorString(void);
    jclass FindClass(const char *name);
    jobject forName(const char *name);
    jobject getPrimitiveClassType(const char *name);
    jmethodID GetMethodID(jclass cls, const char *name, const char *sig);
    jmethodID GetStaticMethodID(jclass cls, const char *name, const char *sig);
    jmethodID GetMethodID(const char *cls, const char *name, const char *sig);
    jmethodID GetStaticMethodID(const char *cls, const char *name, const char *sig);
    std::string jstringToString(jobject str);
    std::string getClassName(jobject cls);
    jvalue CallMethodA(const char retType, jobject obj, jmethodID method, jvalue *args);
    jvalue CallStaticMethodA(const char retType, jclass cls, jmethodID method, jvalue *args);

    void serialize(const Pothos::Proxy &, std::ostream &);
    Pothos::Proxy deserialize(std::istream &);

    JavaVM *jvm;       /* pointer to open virtual machine */
    JNIEnv *env;       /* pointer to native method interface */
};

/***********************************************************************
 * custom java class handler overload
 **********************************************************************/
class JavaProxyHandle : public Pothos::ProxyHandle
{
public:

    JavaProxyHandle(std::shared_ptr<JavaProxyEnvironment> env, jvalue value, char sig);

    Pothos::ProxyEnvironment::Sptr getEnvironment(void) const
    {
        return env;
    }

    Pothos::Proxy call(const std::string &name, const Pothos::Proxy *args, const size_t numArgs);

    int compareTo(const Pothos::Proxy &proxy) const;
    size_t hashCode(void) const;
    std::string toString(void) const;
    std::string getClassName(void) const;

    std::shared_ptr<JavaProxyEnvironment> env;

    //actual value held by the handler:
    //This will be a jclass returned by findObject
    //or something returned by a call (method or constructor).
    jvalue value;

    //character signature type for jvalue: I, Z, J, etc...
    char sig;

    //get character signature from the class Type
    char getSigFromClassType(jobject classType) const;

    //convert internal jvalue which may be a primitive to equivalent jobject
    jobject toJobject(void) const;
};
