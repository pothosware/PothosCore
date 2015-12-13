// Copyright (c) 2013-2013 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object/ObjectMImpl.hpp>
#include <cassert>

Pothos::ObjectM::ObjectM(void)
{
    assert(not *this);
}

Pothos::ObjectM::ObjectM(const ObjectM &obj):
    Object()
{
    *this = obj;
}

Pothos::ObjectM::ObjectM(ObjectM &&obj):
    Object()
{
    *this = obj;
}

Pothos::ObjectM::ObjectM(ObjectM &obj):
    Object()
{
    *this = obj;
}

Pothos::ObjectM::ObjectM(const ObjectM &&obj):
    Object()
{
    *this = obj;
}

Pothos::ObjectM::~ObjectM(void)
{
    return;
}

Pothos::ObjectM &Pothos::ObjectM::operator=(const ObjectM &rhs)
{
    Object::operator=(rhs);
    return *this;
}

Pothos::ObjectM &Pothos::ObjectM::operator=(ObjectM &&rhs)
{
    Object::operator=(std::move(rhs));
    return *this;
}
