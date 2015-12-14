// Copyright (c) 2013-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Serialize.hpp>
#include <Pothos/Object/Exception.hpp>

//required define: portable only exported in pothos-serialization
#define NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include <Pothos/archive/eos/portable_oarchive.hpp>
#include <Pothos/archive/eos/portable_iarchive.hpp>

#include <cassert>

std::ostream &Pothos::Object::serialize(std::ostream &os) const
{
    try
    {
        eos::polymorphic_portable_oarchive oa(os);
        oa << *this;
    }
    catch(const Pothos::archive::archive_exception &ex)
    {
        throw ObjectSerializeError("Pothos::Object::serialize("+this->toString()+")", ex.what());
    }

    return os;
}

std::istream &Pothos::Object::deserialize(std::istream &is)
{
    assert(not *this);

    try
    {
        eos::polymorphic_portable_iarchive ia(is);
        ia >> *this;
    }
    catch(const Pothos::archive::archive_exception &ex)
    {
        throw ObjectSerializeError("Pothos::Object::deserialize()", ex.what());
    }
    //text archive doesn't bounds check and can cause std::bad_alloc
    //for example if the istream is bad after the remove server dies
    catch(const std::exception &ex)
    {
        throw ObjectSerializeError("Pothos::Object::deserialize()", ex.what());
    }

    return is;
}
