///
/// \file Util/UID.hpp
///
/// Unique ID interface class.
///
/// \copyright
/// Copyright (c) 2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <memory>
#include <string>

namespace Pothos {
namespace Util {

/*!
 * Represent a universally unique ID.
 */
class POTHOS_API UID
{
public:

    //! Default constructor
    UID(void);

    //! Get the UID as a string
    const std::string &uid(void) const;

private:
    std::shared_ptr<void> _unique;
    std::string _uid;
};

} //namespace Util
} //namespace Pothos

inline const std::string &Pothos::Util::UID::uid(void) const
{
    return _uid;
}
