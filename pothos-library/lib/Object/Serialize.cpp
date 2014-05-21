// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Serialize.hpp>
#include <Pothos/Object/Exception.hpp>

#include <Pothos/archive/polymorphic_binary_oarchive.hpp>
#include <Pothos/archive/polymorphic_binary_iarchive.hpp>

#include <cassert>

std::ostream &Pothos::Object::serialize(std::ostream &os) const
{
    try
    {
        Pothos::archive::polymorphic_binary_oarchive oa(os);
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
    assert(this->null());

    try
    {
        Pothos::archive::polymorphic_binary_iarchive ia(is);
        ia >> *this;
    }
    catch(const Pothos::archive::archive_exception &ex)
    {
        throw ObjectSerializeError("Pothos::Object::deserialize()", ex.what());
    }

    return is;
}
