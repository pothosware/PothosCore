///
/// \file Object/ObjectMImpl.hpp
///
/// Template implementation details for ObjectM.
///
/// \copyright
/// Copyright (c) 2013-2017 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/ObjectImpl.hpp>
#include <Pothos/Object/ObjectM.hpp>
#include <type_traits>

namespace Pothos {

template <typename ValueType>
ObjectM::ObjectM(ValueType &&value):
    Object(std::forward<ValueType>(value))
{
    return;
}

template <typename ValueType>
ValueType &ObjectM::extract(void) const
{
    return Detail::ObjectContainer::extract<ValueType>(*this);
}

template <typename ValueType>
ObjectM::operator ValueType &(void) const
{
    return this->extract<ValueType>();
}


} //namespace Pothos
