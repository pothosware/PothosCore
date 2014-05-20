//
// Util/RefHolder.hpp
//
// Utility that allows derived objects to maintain references for garbage collection.
//
// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object/Object.hpp>
#include <vector>

namespace Pothos {
namespace Util {

/*!
 * RefHolder stores a list of Objects which are reference counted objects.
 * Classes may derive from RefHolder and use it to store parent objects;
 * to avoid unnecessary lifetime management burdens for the user.
 */
class POTHOS_API RefHolder
{
public:

    //! Virtual destructor for derived classes
    virtual ~RefHolder(void);

    //! Store a copy of this object container
    void holdRef(const Object &container);

    //! Drop a copy of this object container
    void dropRef(const Object &container);

private:
    std::vector<Object> _refs;
};

} //namespace Util
} //namespace Pothos
