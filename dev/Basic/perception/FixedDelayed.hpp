/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <list>

#include "util/LangHelpers.hpp"


namespace sim_mob
{

#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

/**
 * Templatized wrapper for data values with a fixed delay.
 *
 * \author Seth N. Hetu
 * \author Xu Yan
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
	 * \param reclaimPtrs If true, any item discarded by this history list is deleted. Does nothing if the template type is not a pointer.
	 */
	explicit FixedDelayed(size_t delayMS, bool reclaimPtrs=true) : maxDelayMS(delayMS), reclaimPtrs(reclaimPtrs) {}


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
	const T& sense(uint32_t currTimeMS, uint32_t delayMS)
	{
		//Consistency check, done via update.
		if (!can_sense(currTimeMS)) {
			throw std::runtime_error("Can't sense: not enough time has passed.");
		}

		//Return the first value in the list; can_sense() will have already performed the necessary updates.
		//const T& test = history.front().item;


		typename std::list<HistItem>::iterator iter = history.begin();
		while(iter!=history.end())
		{
			HistItem& histItem = *iter;
			if((++iter)==history.end())
				return histItem.item;
			HistItem& histItemNext = *iter;
			if(histItem.canObserve(currTimeMS,delayMS)&&(!histItemNext.canObserve(currTimeMS,delayMS)))
				return histItem.item;
		}
		//if for loop doesn't return
		return history.front().item; //Debugging....
		//return history.front().item;
	}


	/**
	 * Return true if the current time is sufficiently advanced for sensing to occur.
	 * Once this function returns true at least once, it will always return true (until clear() is called).
	 */
	bool can_sense(uint32_t currTimeMS) {
		//Loop while the first value is "sense"-able.
		while (!history.empty() && history.front().canObserve(currTimeMS, maxDelayMS)) {
			//If the second element in the list is non-sensable, we're done.
			if (history.size()==1 || !(++history.begin())->canObserve(currTimeMS, maxDelayMS)) {
				return true;
			}

			//Otherwise, remove the first element.
			if (reclaimPtrs) {
				delete_possible_pointer(history.front().item);
				history.pop_front();
			}
		}

		//In this case, nothing can be observed
		return false;
	}




private:
	//Internal class to store an item and its observed time.
	struct HistItem {
		T item;
		uint32_t observedTime;

		HistItem(T item, uint32_t observedTime) : item(item), observedTime(observedTime) {}

		bool canObserve(uint32_t currTimeMS, size_t delayMS){
			return observedTime + delayMS <= currTimeMS;
		}

		//add by xuyan
		#ifndef SIMMOB_DISABLE_MPI
		public:
			friend class sim_mob::PackageUtils;
			friend class sim_mob::UnPackageUtils;
		#endif
	};

	//Private data
	std::list<HistItem> history;
	size_t maxDelayMS;
	bool reclaimPtrs;


#ifndef SIMMOB_DISABLE_MPI
public:
	friend class sim_mob::PackageUtils;
	friend class sim_mob::UnPackageUtils;
#endif

};

}
