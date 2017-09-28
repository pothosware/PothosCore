// Copyright (c) 2015-2017 Josh Blum
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
        if (dtype.isComplex() and dtype.isFloat())
        {
            const std::complex<double> c = *this;
            if (c.imag() == 0.0) return std::to_string(c.real());
            if (c.real() == 0.0) return std::to_string(c.imag())+"j";
            if (c.imag() < 0.0) return Poco::format("%f-%fj", c.real(), -c.imag());
            return Poco::format("%f+%fj", c.real(), c.imag());
        }
        if (dtype.isComplex())
        {
            const std::complex<long long> c = *this;
            if (c.imag() == 0) return std::to_string(c.real());
            if (c.real() == 0) return std::to_string(c.imag())+"j";
            if (c.imag() < 0) return Poco::format("%s-%sj", std::to_string(c.real()), std::to_string(-c.imag()));
            return Poco::format("%s+%sj", std::to_string(c.real()), std::to_string(c.imag()));
        }
        if (dtype.isFloat()) return std::to_string(double(*this));
        if (dtype.isSigned()) return std::to_string((long long)(*this));
        return std::to_string((unsigned long long)(*this));
    }
    catch (...) {}

    //try object vector
    if (this->canConvert(typeid(Pothos::ObjectVector))) try
    {
        std::string out = "[";
        for (const auto &obj_i : this->operator Pothos::ObjectVector())
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
        for (const auto &pair : this->operator Pothos::ObjectMap())
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
