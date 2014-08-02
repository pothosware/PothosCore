//
// Framework/Connectable.hpp
//
// This file contains the connnectable interface for topology elements.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Util/UID.hpp>
#include <Pothos/Util/RefHolder.hpp>
#include <string>
#include <vector>

namespace Pothos {

//! Information about an IO port
class POTHOS_API PortInfo
{
public:
    PortInfo(void);

    std::string name; //!< The port name

    bool isSigSlot; //!< Is signal or slot?

    DType dtype; //!< The data type
};

/*!
 * Connectable interface for topological elements.
 */
class POTHOS_API Connectable : public Util::UID, public Util::RefHolder
{
public:

    //! virtual destructor for inheritance
    virtual ~Connectable(void);

    /*!
     * Set a displayable name for this topology element.
     * The name is a string to be used in debug outputs.
     * \param name a new displayable name
     */
    void setName(const std::string &name);

    /*!
     * Get the displayable name for this topology element.
     * \return the displayable name string
     */
    const std::string &getName(void) const;

    /*!
     * Get a vector of info about all of the input ports available.
     */
    virtual std::vector<PortInfo> inputPortInfo(void) = 0;

    /*!
     * Get a vector of info about all of the output ports available.
     */
    virtual std::vector<PortInfo> outputPortInfo(void) = 0;

    /*!
     * Get the names of the input ports available.
     */
    std::vector<std::string> inputPortNames(void);

    /*!
     * Get the names of the output ports available.
     */
    std::vector<std::string> outputPortNames(void);

private:
    std::string _name;
};

} //namespace Pothos
