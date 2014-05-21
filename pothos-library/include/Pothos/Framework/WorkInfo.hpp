//
// Framework/WorkInfo.hpp
//
// WorkInfo provides information about a Worker's work() session.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Config.hpp>
#include <cstddef> //size_t
#include <vector>

namespace Pothos {

/*!
 * Information about a work session.
 * The worker can query this info from calls to work().
 * This information is not specific to a single port.
 */
struct POTHOS_API WorkInfo
{
    //! Default constructor -- zeros out members
    WorkInfo(void);

    /*!
     * A vector of input pointers for indexable ports.
     * inputPointers[i] == worker->input(i).buffer.as<const void>();
     * This is a convenience for APIs that use a vector of pointers.
     */
    std::vector<const void *> inputPointers;

    /*!
     * A vector of output pointers for indexable ports.
     * outputPointers[i] == worker->output(i).buffer.as<void>();
     * This is a convenience for APIs that use a vector of pointers.
     */
    std::vector<void *> outputPointers;

    //! The minimum number of elements of all indexed ports
    size_t minElements;

    //! The minimum number of elements of input indexed ports
    size_t minInElements;

    //! The minimum number of elements of output indexed ports
    size_t minOutElements;

    //! The minimum number of elements of all ports
    size_t minAllElements;

    //! The minimum number of elements of input ports
    size_t minAllInElements;

    //! The minimum number of elements of output ports
    size_t minAllOutElements;

    /*!
     * The maximum time a call in work() is allowed to block.
     * The maxTimeoutNs member is in units of nanoseconds.
     * This is for workers that use blocking calls like select.
     * Always use blocking calls with a timeout to be safe.
     */
    long long maxTimeoutNs;
};

} //namespace Pothos
