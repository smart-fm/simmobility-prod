/*
 * ScreenLineCounter.hpp
 *
 *  Created on: 8 Jun, 2015
 *      Author: balakumar
 */

#pragma once

#include "entities/Agent.hpp"
#include "Common.hpp"
#include <boost/thread/mutex.hpp>

namespace sim_mob{
	class ScreenLineCounter {
	public:
		/*
		 * Function to get the singleton instance
		 * return - Singleton Instance
		 */
		static ScreenLineCounter* getInstance();

		/*
		 * Update the count of vehicles passing through a screen line segment
		 * @param rdSegStat Road Segment statistics
		 */
		void updateScreenLineCount(const sim_mob::Agent::RdSegTravelStat& rdSegStat);

		/*
		 * Export the screen line count to a file.
		 */
		void exportScreenLineCount();
	private:
		/*
		 * Constructor
		 */
		ScreenLineCounter();

		/*
		 * Destructor
		 */
		virtual ~ScreenLineCounter();

		/*
		 * Get the Time Interval based on user configuration
		 */
		double getTimeInterval(const double time);

		/**
		 *	container to store road segment travel times at different time intervals
		 */
		sim_mob::TravelTime ttMap;

		/**
		 * time interval value used for processing data.
		 * This value is based on its counterpart in pathset manager.
		 */
		unsigned int intervalMS;

		/*
		 * List of Screen Lines
		 */
		std::vector<unsigned long> screenLines;

		/*
		 * Singleton Instance
		 */
		static ScreenLineCounter* instance;
		static boost::mutex instanceMutex;
	};
}

