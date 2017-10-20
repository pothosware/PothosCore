// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Managed/Exception.hpp>
#include <Pothos/Managed/Class.hpp>
#include <Pothos/Managed/ClassImpl.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/Format.h>
#include <string>
#include <map>
#include <vector>
#include <cctype> //isalnum, tolower

struct Pothos::ManagedClass::Impl
{
    Pothos::Callable referenceToWrapper;
    Pothos::Callable pointerToWrapper;
    Pothos::Callable sharedToWrapper;

    std::vector<Pothos::Callable> baseConverters;
    std::vector<Pothos::Callable> constructors;
    std::map<std::string, std::vector<Pothos::Callable>> staticMethods;
    std::map<std::string, std::vector<Pothos::Callable>> methods;

    Pothos::Callable opaqueConstructor;
    std::map<std::string, Pothos::Callable> opaqueStaticMethods;
    Pothos::Callable wildcardStaticMethod;

    std::map<std::string, Pothos::Callable> opaqueMethods;
    Pothos::Callable wildcardMethod;
};

Pothos::ManagedClass::ManagedClass(void):
    _impl(new Impl())
{
    return;
}

const std::type_info &Pothos::ManagedClass::type(void) const
{
    if (not _impl->referenceToWrapper)
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::type()", "cant access without constructors");
    }
    return _impl->referenceToWrapper.type(0);
}

const std::type_info &Pothos::ManagedClass::pointerType(void) const
{
    if (not _impl->pointerToWrapper)
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::pointerType()", "cant access without init");
    }
    return _impl->pointerToWrapper.type(0);
}

const std::type_info &Pothos::ManagedClass::sharedType(void) const
{
    if (not _impl->sharedToWrapper)
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::pointerType()", "cant access without init");
    }
    return _impl->sharedToWrapper.type(0);
}

Pothos::ManagedClass &Pothos::ManagedClass::registerReferenceToWrapper(const Callable &toWrapper)
{
    if (_impl->referenceToWrapper and toWrapper.type(0) != _impl->referenceToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerReferenceToWrapper()", "class type mismatch");
    }
    _impl->referenceToWrapper = toWrapper;
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerPointerToWrapper(const Callable &toWrapper)
{
    if (_impl->pointerToWrapper and toWrapper.type(0) != _impl->pointerToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerPointerToWrapper()", "class type mismatch");
    }
    _impl->pointerToWrapper = toWrapper;
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerSharedToWrapper(const Callable &toWrapper)
{
    if (_impl->sharedToWrapper and toWrapper.type(0) != _impl->sharedToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerSharedToWrapper()", "class type mismatch");
    }
    _impl->sharedToWrapper = toWrapper;
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerToBaseClass(const Callable &toBase)
{
    if (_impl->referenceToWrapper and toBase.type(0) != _impl->referenceToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerToBaseClass()", "class type mismatch");
    }
    _impl->baseConverters.push_back(toBase);
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerConstructor(const Callable &constructor)
{
    if (_impl->referenceToWrapper and constructor.type(-1) != _impl->referenceToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerConstructor()", "class type mismatch");
    }
    _impl->constructors.push_back(constructor);
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerStaticMethod(const std::string &name, const Callable &method)
{
    _impl->staticMethods[name].push_back(method);
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerMethod(const std::string &name, const Callable &method)
{
    if (_impl->referenceToWrapper and method.getNumArgs() > 0 and method.type(0) != _impl->referenceToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerMethod()", "class type mismatch");
    }
    _impl->methods[name].push_back(method);
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerOpaqueConstructor(const Callable &constructor)
{
    if (_impl->referenceToWrapper and constructor.type(-1) != _impl->referenceToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerOpaqueConstructor()", "class type mismatch");
    }
    if (constructor.getNumArgs() != 2 or constructor.type(0) != typeid(const Object *) or constructor.type(1) != typeid(size_t))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerOpaqueConstructor()", "opaque args incorrect");
    }
    _impl->opaqueConstructor = constructor;
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerOpaqueStaticMethod(const std::string &name, const Callable &method)
{
    if (method.type(-1) != typeid(Object) or method.getNumArgs() != 2 or method.type(0) != typeid(const Object *) or method.type(1) != typeid(size_t))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerOpaqueStaticMethod("+name+")", "opaque args incorrect");
    }
    _impl->opaqueStaticMethods[name] = method;
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerWildcardStaticMethod(const Callable &method)
{
    if (method.type(-1) != typeid(Object) or method.getNumArgs() != 3 or method.type(0) != typeid(std::string) or method.type(1) != typeid(const Object *) or method.type(2) != typeid(size_t))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerWildcardStaticMethod()", "wildcard args incorrect");
    }
    _impl->wildcardStaticMethod = method;
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerOpaqueMethod(const std::string &name, const Callable &method)
{
    if (_impl->referenceToWrapper and method.getNumArgs() > 0 and method.type(0) != _impl->referenceToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerOpaqueMethod()", "class type mismatch");
    }
    if (method.type(-1) != typeid(Object) or method.getNumArgs() != 3 or method.type(1) != typeid(const Object *) or method.type(2) != typeid(size_t))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerOpaqueMethod("+name+")", "opaque args incorrect");
    }
    _impl->opaqueMethods[name] = method;
    return *this;
}

Pothos::ManagedClass &Pothos::ManagedClass::registerWildcardMethod(const Callable &method)
{
    if (_impl->referenceToWrapper and method.getNumArgs() > 0 and method.type(0) != _impl->referenceToWrapper.type(0))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerWildcardMethod()", "class type mismatch");
    }
    if (method.type(-1) != typeid(Object) or method.getNumArgs() != 4 or method.type(1) != typeid(std::string) or method.type(2) != typeid(const Object *) or method.type(3) != typeid(size_t))
    {
        throw ManagedClassTypeError("Pothos::ManagedClass::registerWildcardMethod()", "wildcard args incorrect");
    }
    _impl->wildcardMethod = method;
    return *this;
}

static std::string typeToPluginPath(const std::type_info &type)
{
    std::string out;
    for (char ch : Pothos::Util::typeInfoToString(type))
    {
        if (std::isalnum(ch)) out.push_back(std::tolower(ch));
        else out.push_back('_');
    }
    return out;
}

Pothos::ManagedClass &Pothos::ManagedClass::commit(const std::string &classPath)
{
    //register conversions for constructors that take one argument
    for (const auto &constructor : this->getConstructors())
    {
        if (constructor.getNumArgs() != 1) continue;
        Pothos::PluginRegistry::add(
            Poco::format("/object/convert/constructors/%s_to_%s",
            typeToPluginPath(constructor.type(0)),
            typeToPluginPath(constructor.type(-1))),
            constructor
        );
    }

    PluginRegistry::add(
        PluginPath("/managed").join(classPath),
        dynamic_cast<ManagedClass &>(*this));
    return *this;
}

void Pothos::ManagedClass::unload(const std::string &classPath)
{
    //extract the managed class from the plugin tree
    auto plugin = PluginRegistry::get(PluginPath("/managed").join(classPath));
    const ManagedClass &managedCls = plugin.getObject();

    //unload conversion constructors
    for (const auto &constructor : managedCls.getConstructors())
    {
        if (constructor.getNumArgs() != 1) continue;
        Pothos::PluginRegistry::remove(
            Poco::format("/object/convert/constructors/%s_to_%s",
            typeToPluginPath(constructor.type(0)),
            typeToPluginPath(constructor.type(-1)))
        );
    }

    //then unload the managed class
    Pothos::PluginRegistry::remove(PluginPath("/managed").join(classPath));
}

const Pothos::Callable &Pothos::ManagedClass::getReferenceToWrapper(void) const
{
    return _impl->referenceToWrapper;
}

const Pothos::Callable &Pothos::ManagedClass::getPointerToWrapper(void) const
{
    return _impl->pointerToWrapper;
}

const Pothos::Callable &Pothos::ManagedClass::getSharedToWrapper(void) const
{
    return _impl->sharedToWrapper;
}

const std::vector<Pothos::Callable> &Pothos::ManagedClass::getBaseClassConverters(void) const
{
    return _impl->baseConverters;
}

const std::vector<Pothos::Callable> &Pothos::ManagedClass::getConstructors(void) const
{
    return _impl->constructors;
}

const std::vector<Pothos::Callable> &Pothos::ManagedClass::getStaticMethods(const std::string &name) const
{
    auto it = _impl->staticMethods.find(name);
    if (it != _impl->staticMethods.end()) return it->second;
    throw ManagedClassNameError("Pothos::ManagedClass::getStaticMethods("+name+")", "name not found");
}

const std::vector<Pothos::Callable> &Pothos::ManagedClass::getMethods(const std::string &name) const
{
    auto it = _impl->methods.find(name);
    if (it != _impl->methods.end()) return it->second;
    throw ManagedClassNameError("Pothos::ManagedClass::getMethods("+name+")", "name not found");
}

const Pothos::Callable &Pothos::ManagedClass::getOpaqueConstructor(void) const
{
    return _impl->opaqueConstructor;
}

const Pothos::Callable &Pothos::ManagedClass::getOpaqueStaticMethod(const std::string &name) const
{
    auto it = _impl->opaqueStaticMethods.find(name);
    if (it != _impl->opaqueStaticMethods.end()) return it->second;
    throw ManagedClassNameError("Pothos::ManagedClass::getOpaqueStaticMethod("+name+")", "name not found");
}

const Pothos::Callable &Pothos::ManagedClass::getWildcardStaticMethod(void) const
{
    return _impl->wildcardStaticMethod;
}

const Pothos::Callable &Pothos::ManagedClass::getOpaqueMethod(const std::string &name) const
{
    auto it = _impl->opaqueMethods.find(name);
    if (it != _impl->opaqueMethods.end()) return it->second;
    throw ManagedClassNameError("Pothos::ManagedClass::getWildcardStaticMethod("+name+")", "name not found");
}

const Pothos::Callable &Pothos::ManagedClass::getWildcardMethod(void) const
{
    return _impl->wildcardMethod;
}
