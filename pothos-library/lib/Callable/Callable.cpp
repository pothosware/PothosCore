// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Callable/CallableImpl.hpp>
#include <Pothos/Callable/Exception.hpp>
#include <Pothos/Object/Exception.hpp>
#include <Pothos/Util/TypeInfo.hpp>
#include <Poco/Format.h>
#include <cassert>

#define MAX_SUPPORTED_NUM_ARGS 13

Pothos::Callable::Callable(void)
{
    assert(this->null());
}

Pothos::Object Pothos::Callable::opaqueCall(const Object *inputArgs, const size_t numArgs) const
{
    if (_impl == nullptr)
    {
        throw Pothos::CallableNullError("Pothos::Callable::call()", "null Callable");
    }

    const size_t numCallArgs = _impl->getNumArgs();
    if (numCallArgs > MAX_SUPPORTED_NUM_ARGS)
    {
        throw Pothos::CallableNullError("Pothos::Callable::call()", "more args than supported");
    }

    //Create callArgs which is a combination of inputArgs and boundArgs
    Object callArgs[MAX_SUPPORTED_NUM_ARGS];
    size_t inputArgsIndex = 0;
    for (size_t i = 0; i < numCallArgs; i++)
    {
        //is there a binding? if so use it
        if (_boundArgs.size() > i and not _boundArgs[i].null())
        {
            callArgs[i] = _boundArgs[i];
        }

        //otherwise, use the next available input argument
        else
        {
            if (numArgs <= inputArgsIndex)
            {
                throw Pothos::CallableArgumentError("Pothos::Callable::call()", Poco::format(
                    "expected input argument at %d", int(inputArgsIndex)));
            }
            callArgs[i] = inputArgs[inputArgsIndex++];
        }

        //perform conversion on arg to get an Object of the exact type
        try
        {
            callArgs[i] = callArgs[i].convert(_impl->type(int(i)));
        }
        catch(const Pothos::ObjectConvertError &ex)
        {
            throw Pothos::CallableArgumentError("Pothos::Callable::call()", Poco::format(
                "failed to convert arg%d\n%s", int(i), std::string(ex.displayText())));
        }
    }

    return _impl->call(callArgs);
}

size_t Pothos::Callable::getNumArgs(void) const
{
    if (_impl == nullptr)
    {
        throw Pothos::CallableNullError("Pothos::Callable::getNumArgs()", "null Callable");
    }

    size_t numArgs = _impl->getNumArgs();

    //remove bound args from the count
    for (size_t i = 0; i < std::min(numArgs, _boundArgs.size()); i++)
    {
        if (not _boundArgs[i].null()) numArgs--;
    }

    return numArgs;
}

const std::type_info &Pothos::Callable::type(const int argNo) const
{
    if (_impl == nullptr)
    {
        throw Pothos::CallableNullError("Pothos::Callable::type()", "null Callable");
    }

    if (argNo < -1)
    {
        throw Pothos::CallableArgumentError("Pothos::Callable::type()", Poco::format(
            "unexpected argNo %d", int(argNo)));
    }
    if (argNo >= int(this->getNumArgs()))
    {
        throw Pothos::CallableArgumentError("Pothos::Callable::type()", Poco::format(
            "unexpected argNo %d but call takes %d arguments", int(argNo), int(this->getNumArgs())));
    }

    if (argNo == -1) return _impl->type(-1);

    //add bound args into the actual arg number
    int skippedIndexes = 0;
    for (size_t i = 0; i < _boundArgs.size(); i++)
    {
        if (not _boundArgs[i].null()) skippedIndexes++;
        else if (int(i) == argNo + skippedIndexes) break;
    }

    return _impl->type(argNo + skippedIndexes);
}

Pothos::Callable &Pothos::Callable::bind(Object &&val, const size_t argNo)
{
    if (_boundArgs.size() <= argNo) _boundArgs.resize(argNo+1);
    _boundArgs[argNo] = val;
    return *this;
}

Pothos::Callable &Pothos::Callable::unbind(const size_t argNo)
{
    return this->bind(Object(), argNo);
}

std::string Pothos::Callable::toString(void) const
{
    if (_impl == nullptr) return "null";

    std::string output;
    output += Util::typeInfoToString(this->type(-1));
    output += "(";
    for (size_t i = 0; i < this->getNumArgs(); i++)
    {
        if (i != 0) output += ", ";
        output += Util::typeInfoToString(this->type(int(i)));
    }
    output += ")";
    return output;
}

Pothos::Detail::CallableContainer::CallableContainer(void)
{
    return;
}

Pothos::Detail::CallableContainer::~CallableContainer(void)
{
    return;
}

bool Pothos::Callable::null(void) const
{
    return not _impl;
}

bool Pothos::operator==(const Callable &lhs, const Callable &rhs)
{
    return (lhs._impl == rhs._impl) and (lhs._boundArgs == rhs._boundArgs);
}
