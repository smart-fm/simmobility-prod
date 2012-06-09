/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include "BufferedDataManager.hpp"

#include <boost/thread.hpp>


namespace sim_mob
{


///Strategy for enforcing mutual exclusion
/// \author Seth N. Hetu
enum MutexStrategy {
	MtxStrat_Buffered,
	MtxStrat_Locked,
};



/**
 * Templatized wrapper for an object that may be Buffered OR Locked.
 *
 * \author Seth N. Hetu
 * \author Xu Yan
 *
 * A Buffered datum handle multiple readers and a single writer without
 * locking. The "flip" method is used to update the current value after calling "set".
 *
 * A Locked datum handle multiple readers and a single writer through locking. The "set" and "get"
 *  methods ensure mutual exclusion. The "flip" method is an artifact of BufferedBase and does nothing.
 *
 * The spoken semantics of this template are sensible; for example:
 *   Shared<int>
 * ...is a "Shared int".
 *
 *  \note
 *  This class is basically a merge of Locked<> and Buffered<>. I created it because I
 *  would like to be able to test Buffered<> performance by switching between Locked/Buffered
 *  types at runtime (using a config option). It is possible to have Shared<Buffered<T>> and
 *  Shared<Locked<T>>, but that will only be resolved at compile time. We can dedice later
 *  if we want to keep Shared<> or if it's better to use Locked/Buffered, so please do not
 *  delete these other classes for now.
 *
 *  \note
 *  If you are getting weird errors about "::move" being ambiguous, you might be using an older
 *  version of boost 1.48. You can upgrade boost, or you can set SIMMOB_LATEST_STANDARD to true
 *  to fix this problem. There is also a patch for boost 1.48 that fixes this:
 *  https://svn.boost.org/trac/boost/ticket/6141#comment:10
 *
 *  \par
 *  The Locked<> type is written with an "Upgradable Lock", which should be fast for multiple
 *  reads and one write.
 *
 *   \par
 *   ~Seth
 */
template <typename T>
class Shared : public BufferedBase
{
public:
	/**
	 * Create a new Shared data type.
	 *
	 * \param value The initial value. You can also set an initial value using "force".
	 */
	Shared (const sim_mob::MutexStrategy& mtxStrategy, const T& value = T()) : BufferedBase(),
		current_ (value), strategy_(mtxStrategy), next_ (value) {}
	virtual ~Shared() {}


	/**
	 * Retrieve the current value. Get the current value of the data type. This can
	 * also be thought of as being one flip "behind" the actual value.
	 */
    const T& get() const {
    	if (strategy_==MtxStrat_Locked) {
    		boost::shared_lock<boost::shared_mutex> lock_(mutex_);
    		return current_;
    	}
    }

	/**
	 * Set the next value. Set the next value of the data type. This value will
	 * only take effect when "flip" is called.
	 */
    void set (const T& value) {
    	if (strategy_==MtxStrat_Locked) {
        	boost::upgrade_lock<boost::shared_mutex> lock_(mutex_);
        	boost::upgrade_to_unique_lock<boost::shared_mutex> unique_(lock_);
        	current_ = value;
    	} else if (strategy_==MtxStrat_Buffered) {
    		next_ = value;
    	}
    }


	/**
	 * Skips processing for this time tick. If an agent won't be updating a particular
	 * Buffered type during its time tick, it should call skip() on that type.
	 *
	 * \note
	 * This is intended for later, when we have pointers to arrays of data to update.
	 * But modelers should definitely respect the limitations of Buffere<> types now.
	 */
    void skip() {
    	const T& val = this->get();
    	this->set(val);
    }


    /**
     * Evaluates as the current value.
     *
     * Used in an expression, the object will be evaluated as its current value.  Example:
     *     \code
     *     enum Color { GREEN, YELLOW, RED };
     *     Buffered<Color> color;
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
     *     Buffered<uint32_t> passenger_count;
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

	/**
	 * Force a new value into effect. Set the current and next value without a call to flip/lock().
	 * This is usually only needed when loading values from a config file.
	 *
	 * Note that calling this function is inherently unsafe in a parallel environment.
	 */
    void force(const T& value) {
    	current_ = value;
    	if (strategy_==MtxStrat_Buffered) {
    		next_ = value;
    	}
    }

protected:
    void flip() {
    	if (strategy_==MtxStrat_Buffered) {
    		current_ = next_;
    	}
    }

    //Used by both
    T current_;

    sim_mob::MutexStrategy strategy_;

    //Next value to be written
    // Used by Buffered
    T next_;

    //Shared ownership of reading, exclusive ownership of writing.
    // Used by Locked
    mutable boost::shared_mutex mutex_;


};

}




