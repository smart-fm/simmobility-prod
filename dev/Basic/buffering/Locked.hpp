/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "BufferedDataManager.hpp"

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>


namespace sim_mob
{


/**
 * Templatized wrapper for locked objects.
 *
 * A Locked datum handle multiple readers and a single writer through locking. The "set" and "get"
 *  methods ensure mutual exclusion. The "flip" method is an artifact of BufferedBase and does nothing.
 *
 * The spoken semantics of this template are sensible; for example:
 *   Locked<int>
 * ...is a "Locked int".
 *
 * \note
 * Do not delete this class until we decide what to do with it. See Shared.hpp for more information.
 *
 *  \todo It seems that Locked<primitive> shouldn't have to lock on get(), but I'll have to look into this.
 *
 *   \par
 *   I extended BufferedBase so that Buffered and Locked objects are effectively indistinguishable. Currently,
 *   we will definitely use locked types in the mid-term simulator. But a modeler might want to use a Locked
 *   type in the short-term for, say, fast, infrequent inter-agent communication that shouldn't be delayed a turn.
 *   In this case, the engine should allow it.
 *
 *   \par
 *   Note that assigning a Locked class to a BufferedDataManager is wasteful, but effectively harmless.
 *
 *   \par
 *   ~Seth
 */
template <typename T>
class Locked : public BufferedBase
{
public:
	/**
	 * Create a new Locked data type.
	 *
	 * @param value The initial value.
	 */
	explicit Locked (const T& value = T()) : BufferedBase(), current_ (value) {}
	virtual ~Locked() {}


	/**
	 * Retrieve the current value.
	 */
    const T& get() const {
    	boost::shared_lock<boost::shared_mutex> lock_(mutex_);

    	return current_;
    }

	/**
	 * Set the current value.
	 */
    void set (const T& value) {
    	boost::upgrade_lock<boost::shared_mutex> lock_(mutex_);
    	boost::upgrade_to_unique_lock<boost::shared_mutex> unique_(lock_);

    	current_ = value;
    }


    /**
     * Evaluates as the current value.
     *
     * Used in an expression, the object will be evaluated as its current value.  Example:
     *     \code
     *     enum Color { GREEN, YELLOW, RED };
     *     Locked<Color> color;
     *     ...
     *     switch (color)
     *     {
     *     case GREEN: ... break;
     *     case YELLOW: ... break;
     *     case RED: ... break;
     *     }
     *     \endcode
     *
     * Another example:
     *     \code
     *     Locked<uint32_t> passenger_count;
     *     ...
     *     if (passenger_count == 10) ...
     *     std::cout << "There are " << passenger_count << " passengers on the bus\n";
     *     \endcode
     *
     *  Take care that this conversion is not called when \p T is big struct.
     */
    operator T() const
    {
        return get();
    }


protected:
    //Included for compatibility with BufferedBase. Does nothing.
    void flip() {}

    //Shared ownership of reading, exclusive ownership of writing.
    mutable boost::shared_mutex mutex_;

    //Value
    T current_;

};

}




