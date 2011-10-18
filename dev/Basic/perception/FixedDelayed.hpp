/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <list>
#include "../constants.h"


namespace sim_mob
{



/**
 * Templatized wrapper for data values with a fixed delay.
 *
 * Imposes a fixed delay (in ms) on a given object. For example, a FixedDelayed<int> is a
 * "(fixed) delayed integer". Attempting to retrieve the value of a FixedDelayed item will
 * return its most recently observed value (after a delay has been accounted for).
 *
 * \note
 * The FixedDelayed class copies items frequently, so if you are storing a large class or
 * non-copyable item, you should pass a pointer as the template paramter.
 */
template <typename T>
class FixedDelayed {
public:
	/**
	 * Construct a new FixedDelayed item with the given delay in ms.
	 * \param delayMS The time to delay each sensation.
	 */
	FixedDelayed(size_t delayMS) : delayMS(delayMS) {}


	/**
	 * Delay a value, to be returned after the delay time.
	 * \param value The value to delay.
	 * \param currTimeMS The time at which this value physically occurred. It will be sense-able after currTimeMS + delayMS
	 *
	 * \note
	 * The value of currTimeMS must always increase monotonically, or an exception is thrown.
	 */
	void delay(const T& value, unsigned int currTimeMS) {
		//Check consistency
		if (!history.empty() && history.back().observedTime>currTimeMS) {
			throw std::runtime_error("Delay called with an out-of-order currTimeMS.");
		}

		//Save it
		history.push_back(HistItem(value, currTimeMS));
	}


private:
	struct HistItem {
		T item;
		unsigned int observedTime;
	};

	std::list<HistItem> history;
	const size_t delayMS;

};







}
