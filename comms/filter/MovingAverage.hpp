// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/RingDeque.hpp>

/*!
 * Moving average delay line processing unit.
 * \see http://www.digitalsignallabs.com/dcblock.pdf (figure 3b)
 */
template <typename Type>
class MovingAverage
{
public:
    MovingAverage(void):
        D(0),
        b1(0)
    {
        return;
    }

    //! Resize initializes and resets the state.
    void resize(const size_t d)
    {
        D = d;
        b1 = 0;
        hist.clear();
        hist.set_capacity(d);
        while (not hist.full())
        {
            hist.push_back(Type(0));
        }
    }

    /*!
     * Operator feeds an input into the moving average
     * and returns the out value for this input cycle.
     */
    Type operator()(const Type &x)
    {
        //process this cycle
        const auto b0 = (x - this->front()) + b1;

        //stash into hist
        b1 = b0;
        hist.pop_front();
        hist.push_back(x);

        return b0/D;
    }

    //! Access the oldest input in the history.
    Type front(void) const
    {
        return hist.front();
    }

private:
    Type D;
    Type b1;
    Pothos::Util::RingDeque<Type> hist;
};
