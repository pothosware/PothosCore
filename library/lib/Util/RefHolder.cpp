// Copyright (c) 2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/RefHolder.hpp>
#include <algorithm>

Pothos::Util::RefHolder::~RefHolder(void)
{
    _refs.clear();
}

void Pothos::Util::RefHolder::holdRef(const Object &container)
{
    this->dropRef(container); //dont want multiple copies
    _refs.push_back(container);
}

void Pothos::Util::RefHolder::dropRef(const Object &container)
{
    auto it = std::find(_refs.begin(), _refs.end(), container);
    if (it != _refs.end()) _refs.erase(it);
}
