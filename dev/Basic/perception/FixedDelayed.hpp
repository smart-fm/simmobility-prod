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
 *
 * \note
 * This class should be used for continuous values. It should NOT be used for things like
 * reactionary signals or threshold values, since a call to sense() may discard values if they
 * do not represent the latest data.
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
	 * Remove all delayed perceptions.
	 */
	void clear() {
		history.clear();
	}


	/**
	 * Delay a value, to be returned after the delay time.
	 * \param value The value to delay.
	 * \param observedTimeMS The time at which this value physically occurred. It will be sense-able after observedTimeMS + delayMS
	 *
	 * \note
	 * The value of observedTimeMS must always increase monotonically, or an exception is thrown.
	 */
	void delay(const T& value, uint32_t observedTimeMS) {
		//Check consistency
		if (!history.empty() && history.back().observedTime>observedTimeMS) {
			throw std::runtime_error("Delay called with an out-of-order observedTimeMS.");
		}

		//Save it
		history.push_back(HistItem(value, observedTimeMS));
	}


	/**
	 * Retrieve the current perceived value of this item.
	 *
	 * \param currTimeMS The current time of the simulation.
	 *
	 * \note
	 * currTimeMS must be monotonically increasing; sense() is used to retrieve values at
	 * the current time tick, not any arbitrary time tick.
	 */
	const T& sense(uint32_t currTimeMS) {
		//Consistency check, done via update.
		if (!can_sense(currTimeMS)) {
			throw std::runtime_error("Can't sense: not enough time has passed.");
		}

		//Return the first value in the list; can_sense() will have already performed the necessary updates.
		return history.front().item;
	}


	/**
	 * Return true if the current time is sufficiently advanced for sensing to occur.
	 * Once this function returns true at least once, it will always return true (until clear() is called).
	 */
	bool can_sense(uint32_t currTimeMS) {
		//Loop while the first value is "sense"-able.
		while (!history.empty() && history.front().canObserve(currTimeMS)) {
			//If the second element in the list is non-sensable, we're done.
			if (history.size()==1 || !history.at(1).canObserve(currTimeMS)) {
				return true;
			}

			//Otherwise, remove the first element.
			history.pop_front();
		}

		//In this case, nothing can be observed
		return false;
	}


private:
	struct HistItem {
		T item;
		uint32_t observedTime;

		bool canObserve(uint32_t currTimeMS){
			return observedTime + delayMS <= currTimeMS;
		}
	};

	std::list<HistItem> history;
	const size_t delayMS;

};







}
