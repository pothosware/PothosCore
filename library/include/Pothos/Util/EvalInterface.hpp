///
/// \file Util/EvalInterface.hpp
///
/// An exported interface to code to for doing runtime evaluations.
///
/// \copyright
/// Copyright (c) 2014-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Object.hpp>

namespace Pothos {
namespace Util {

/*!
 * A simple interface for retrieving an evaluation result.
 */
class POTHOS_API EvalInterface
{
public:
    EvalInterface(void);
    virtual ~EvalInterface(void);

    //! Get the result of the evaluation
    virtual Object eval(void) const = 0;
};

} //namespace Util
} //namespace Pothos
