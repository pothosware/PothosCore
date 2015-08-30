#ifndef POTHOS_ARCHIVE_ITERATORS_DATAFLOW_HPP
#define POTHOS_ARCHIVE_ITERATORS_DATAFLOW_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// dataflow.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com . 
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <Pothos/serialization/impl/assert.hpp>

#include <Pothos/serialization/impl/config.hpp> // for BOOST_DEDUCED_TYPENAME

#include <Pothos/serialization/impl/mpl/eval_if.hpp>
#include <Pothos/serialization/impl/mpl/if.hpp>
#include <Pothos/serialization/impl/mpl/bool.hpp>
#include <Pothos/serialization/impl/mpl/apply.hpp>
#include <Pothos/serialization/impl/mpl/plus.hpp>
#include <Pothos/serialization/impl/mpl/int.hpp>

#include <Pothos/serialization/impl/type_traits/is_convertible.hpp>
#include <Pothos/serialization/impl/type_traits/is_base_and_derived.hpp>
#include <Pothos/serialization/impl/type_traits/is_pointer.hpp>
#include <Pothos/serialization/impl/iterator/iterator_adaptor.hpp>
#include <Pothos/serialization/impl/iterator/iterator_traits.hpp>
#include <Pothos/serialization/impl/static_assert.hpp>

namespace Pothos { 
namespace archive {
namespace iterators {

// poor man's tri-state
struct tri_state {
    enum state_enum {
        is_false = false,
        is_true = true,
        is_indeterminant
    } m_state;
    // convert to bool
    operator bool (){
        POTHOS_ASSERT(is_indeterminant != m_state);
        return is_true == m_state ? true : false;
    }
    // assign from bool
    tri_state & operator=(bool rhs) {
        m_state = rhs ? is_true : is_false;
        return *this;
    }
    tri_state(bool rhs) :
        m_state(rhs ? is_true : is_false)
    {}
    tri_state(state_enum state) :
        m_state(state)
    {}
    bool operator==(const tri_state & rhs) const {
        return m_state == rhs.m_state;
    }
    bool operator!=(const tri_state & rhs) const {
        return m_state != rhs.m_state;
    }
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// implement functions common to dataflow iterators
template<class Derived>
class dataflow {
    bool m_eoi;
protected:
    // test for iterator equality
    tri_state equal(const Derived & rhs) const {
        if(m_eoi && rhs.m_eoi)
            return true;
        if(m_eoi || rhs.m_eoi)
            return false;
        return tri_state(tri_state::is_indeterminant);
    }
    void eoi(bool tf){
        m_eoi = tf;
    }
    bool eoi() const {
        return m_eoi;
    }
public:
    dataflow(bool tf) :
        m_eoi(tf)
    {}
    dataflow() : // used for iterator end
        m_eoi(true)
    {}
};

} // namespace iterators
} // namespace archive
} // namespace boost

#endif // BOOST_ARCHIVE_ITERATORS_DATAFLOW_HPP
