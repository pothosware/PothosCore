// Boost.Range library
//
//  Copyright Thorsten Ottosen 2003-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org/libs/range/
//

#ifndef POTHOS_RANGE_SIZE_HPP
#define POTHOS_RANGE_SIZE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif

#include <Pothos/serialization/impl/range/config.hpp>
#include <Pothos/serialization/impl/range/begin.hpp>
#include <Pothos/serialization/impl/range/end.hpp>
#include <Pothos/serialization/impl/range/size_type.hpp>
#include <Pothos/serialization/impl/assert.hpp>

namespace Pothos
{
    namespace range_detail
    {
        template<class SinglePassRange>
        inline POTHOS_DEDUCED_TYPENAME range_size<const SinglePassRange>::type
        range_calculate_size(const SinglePassRange& rng)
        {
            POTHOS_ASSERT( (Pothos::end(rng) - Pothos::begin(rng)) >= 0 &&
                          "reachability invariant broken!" );
            return Pothos::end(rng) - Pothos::begin(rng);
        }
    }

    template<class SinglePassRange>
    inline POTHOS_DEDUCED_TYPENAME range_size<const SinglePassRange>::type
    size(const SinglePassRange& rng)
    {
#if !POTHOS_WORKAROUND(__BORLANDC__, POTHOS_TESTED_AT(0x564)) && \
    !POTHOS_WORKAROUND(__GNUC__, < 3) \
    /**/
        using namespace range_detail;
#endif
        return range_calculate_size(rng);
    }

} // namespace 'boost'

#endif
