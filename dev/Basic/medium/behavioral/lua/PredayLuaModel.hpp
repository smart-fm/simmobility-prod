//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayLuaModel.hpp
 *
 *  Created on: Nov 27, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>
#include "behavioral/params/PersonParams.hpp"
#include "behavioral/params/StopGenerationParams.hpp"
#include "behavioral/params/TimeOfDayParams.hpp"
#include "behavioral/params/TourModeParams.hpp"
#include "behavioral/params/ModeDestinationParams.hpp"
#include "lua/LuaModel.hpp"
#include "behavioral/PredayClasses.hpp"

namespace sim_mob {
namespace medium {

/**
 * Class which communicates with the preday models specified in Lua scripts.
 * This class contains a separate function for each model.
 *
 * \author Harish Loganathan
 */
class PredayLuaModel : public lua::LuaModel {
public:
	PredayLuaModel();
	virtual ~PredayLuaModel();

	/**
	 * Predicts the types of tours and intermediate stops the person is going to make.
	 *
	 * @param personParams object containing person and household related variables
	 * @param dayPattern map to fill up with the prediction result
	 */
	void predictDayPattern(PersonParams& personParams, boost::unordered_map<std::string, bool>& dayPattern) const;

	/**
	 * Predicts the number of tours for each type of tour predicted by the day pattern model.
	 *
	 * @param personParams object containing person and household related variables
	 * @param dayPattern map containing the day pattern of this person
	 */
	void predictNumTours(PersonParams& personParams, boost::unordered_map<std::string, bool>& dayPattern, boost::unordered_map<std::string, int>& numTours) const;

	/**
	 * For each work tour, if the person has a usual work location, this function predicts whether the person goes to his usual location or some other location.
	 *
	 * @param personParams object containing person and household related variables
	 * @param usualWorkParams parameters specific to attends usual work model
	 * @return true if the tour is to a usual work location. false otherwise.
	 */
	bool predictUsualWorkLocation(PersonParams& personParams, UsualWorkParams& usualWorkParams) const;

	/**
	 * Predicts the mode of transportation for tours to usual locations (education or work tours)
	 *
	 * @param personParams object containing person and household related variables
	 * @param tourModeParams parameters specific to tour mode models
	 * @return mode id
	 *         (1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
			    5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi)
	 */
	int predictTourMode(PersonParams& personParams, TourModeParams& tourModeParams) const;

	/**
	 * Predicts the mode and destination for the tour together for tours to unusual locations
	 *
	 * @param personParams object containing person and household related variables
	 * @param tourModeDestinationParams parameters specific to tour mode-destination models
	 *
	 * @return an integer in the range of 1 to 9828 (9 modes * 1092 zones) which represents a combination of a zone (one of 1092) and a mode (one of 9)
	 */
	int predictTourModeDestination(PersonParams& personParams, TourModeDestinationParams& tourModeParams) const;

	/**
	 * Predicts the time window for a tour
	 *
	 * @param personParams object containing person and household related variables
	 * @param tourTimeOfDayParams parameters for the tour time of day model
	 * @param tourType type of the tour for which time window is being predicted
	 *
	 * @return an integer in the range of 1 to 1176 representing a particular time window in the day
	 * 			(1 = (3.25,3.25), 2 = (3.25,3.75), 3 = (3.25, 4.25), ... , 48 = (3.25, 26.75), 49 = (3.75, 3.75), ... , 1176 = (26.75,26.75))
	 */
	int predictTourTimeOfDay(PersonParams& personParams, TourTimeOfDayParams& tourTimeOfDayParams, StopType tourType) const;

	/**
	 * Predicts the next stop for a tour
	 *
	 * @param personParam object containing person and household related variables
	 * @param isgParams object containing parameters specific to intermediate stop generation model
	 */
	int generateIntermediateStop(PersonParams& personParams, StopGenerationParams& isgParams) const;

	/**
	 * Predicts the departure/arrival time for a stop
	 *
	 * @param personParams object containing person and household related variables
	 * @param stopTimeOfDayParams parameters for the stop time of day model
	 *
	 * @return an integer in the range of 1 to 48 representing a particular half-hour time window in the day
	 */
	int predictStopTimeOfDay(PersonParams& personParams, StopTimeOfDayParams& stopTimeOfDayParams) const;


private:
    /**
     * Inherited from LuaModel
     */
    void mapClasses();
};
} // end namespace medium
}//end namespace sim_mob
