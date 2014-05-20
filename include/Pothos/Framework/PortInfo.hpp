//
// Framework/PortInfo.hpp
//
// PortInfo provides information about a port on a worker.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/DType.hpp>
#include <string>

namespace Pothos {

/*!
 * Information about an input or output port.
 */
class POTHOS_API PortInfo
{
public:
    //! Default constructor
    PortInfo(void);

    //! Member contructor
    PortInfo(const std::string &name, const DType &dtype);

    //! The string name representation of this port
    const std::string &name(void) const;

    //! The data type representation of this port
    const DType &dtype(void) const;

private:
    std::string _name;
    DType _dtype;
};

} //namespace Pothos
