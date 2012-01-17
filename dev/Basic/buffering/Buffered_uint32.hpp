/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <stdint.h>
#include "Buffered.hpp"

namespace sim_mob
{

/**
 * The double-buffered data type for unsigned 32bit integers.
 *
 * \author LIM Fung Chai
 *
 * This data type is useful for passenger-counts, vehicle-counts, etc.
 */
class Buffered_uint32 : public Buffered<uint32_t>
{
public:
    explicit Buffered_uint32(uint32_t v = 0)
      : Buffered<uint32_t>(v)
    {
    }

    /**
     * Pre-increment operator.
     *
     * The method does not handle overflows and underflows.  Also note that the return
     * type is void.  The following code fragment will not compile:
     *     \code
     *     Buffered_uint32 passenger_count;
     *     ...
     *     if (++passenger_count == max_capacity) ...
     *     \endcode
     * Since the new value will be available only after a call to flip(), it may lead to
     * incorrect code if the above code was allowed to compile.
     */
    void operator++()
    {
        ++next_;
    }

    /**
     * Post-increment operator.
     *
     * The method does not handle overflows and underflows.  Also note that the return
     * typw is void.  \sa Buffered_uint32::operator++().
     */
    void operator++(int)
    {
        ++next_;
    }

    /**
     * Pre-decrement operator.
     *
     * The method does not handle overflows and underflows.  Also note that the return
     * type is void.  \sa Buffered_uint32::operator++().
     */
    void operator--()
    {
        --next_;
    }

    /**
     * Post-decrement operator.
     *
     * The method does not handle overflows and underflows.  Also note that the return
     * type is void.  \sa Buffered_uint32::operator++().
     */
    void operator--(int)
    {
        --next_;
    }

    /**
     * Increase the object by \p delta.
     *
     * The method does not handle overflows and underflows.  Also note that the return
     * type is void.  \sa Buffered_uint32::operator++().
     */
    void operator+=(int delta)
    {
        next_ += delta;
    }

    /**
     * Decrease the object by \p delta.
     *
     * The method does not handle overflows and underflows.  Also note that the return
     * type is void.  \sa Buffered_uint32::operator++().
     */
    void operator-=(int delta)
    {
        next_ -= delta;
    }
};

}
