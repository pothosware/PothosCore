// Copyright (c) 2013-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/Serialize.hpp>
#include <Pothos/Object/Exception.hpp>
#include <cassert>

std::ostream &Pothos::Object::serialize(std::ostream &os) const
{
    try
    {
        Pothos::Archive::OStreamArchiver oa(os);
        oa << *this;
    }
    catch(const Pothos::ArchiveException &ex)
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
        Pothos::Archive::IStreamArchiver ia(is);
        ia >> *this;
    }
    catch(const Pothos::ArchiveException &ex)
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
