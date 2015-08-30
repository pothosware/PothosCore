// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Object/Containers.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Poco/Format.h>
#include <complex>
#include <cassert>

std::string Pothos::Object::toString(void) const
{
    //boolean
    if (this->type() == typeid(bool)) return this->extract<bool>()?"true":"false";

    //string
    if (this->type() == typeid(std::string)) return Poco::format("\"%s\"", this->extract<std::string>());

    //try numeric types
    try
    {
        const Pothos::DType dtype(this->type());
        if (dtype.isComplex())
        {
            const auto c = this->convert<std::complex<double>>();
            if (c.imag() == 0.0) return std::to_string(c.real());
            if (c.real() == 0.0) return std::to_string(c.imag())+"j";
            if (c.imag() < 0.0) return Poco::format("%f-%fj", c.real(), -c.imag());
            return Poco::format("%f+%fj", c.real(), c.imag());
        }
        if (dtype.isFloat()) return std::to_string(this->convert<double>());
        if (dtype.isSigned()) return std::to_string(this->convert<long long>());
        return std::to_string(this->convert<unsigned long long>());
    }
    catch (...) {}

    //try object vector
    if (this->canConvert(typeid(Pothos::ObjectVector))) try
    {
        std::string out = "[";
        for (const auto &obj_i : this->convert<Pothos::ObjectVector>())
        {
            if (out.size() > 1) out += ", ";
            out += obj_i.toString();
        }
        return out + "]";
    }
    catch (...) {}

    //try object map
    if (this->canConvert(typeid(Pothos::ObjectMap))) try
    {
        std::string out = "{";
        for (const auto &pair : this->convert<Pothos::ObjectMap>())
        {
            if (out.size() > 1) out += ", ";
            out += Poco::format("%s: %s", pair.first.toString(), pair.second.toString());
        }
        return out + "}";
    }
    catch (...) {}

    //fall-back data type
    return Poco::format("<%s>", this->getTypeString());
}
