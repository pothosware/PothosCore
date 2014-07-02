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
#include <Pothos/Util/UID.hpp>
#include <string>
#include <vector>

namespace Pothos {

/*!
 * Connectable interface for topological elements.
 */
class POTHOS_API Connectable : public Util::UID
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
     * Get the names of the input ports available.
     */
    virtual std::vector<std::string> inputPortNames(void) = 0;

    /*!
     * Get the names of the output ports available.
     */
    virtual std::vector<std::string> outputPortNames(void) = 0;

private:
    std::string _name;
};

} //namespace Pothos
