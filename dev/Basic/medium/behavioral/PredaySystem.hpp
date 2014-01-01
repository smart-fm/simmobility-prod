//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)
/*
 * SystemOfModels.hpp
 *
 *  Created on: Nov 7, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>

#include "behavioral/lua/PredayLuaProvider.hpp"
#include "params/PersonParams.hpp"
#include "PredayClasses.hpp"
#include "database/PopulationSqlDao.hpp"
#include "database/dao/MongoDao.hpp"

namespace sim_mob {
namespace medium {

/**
 * Class for pre-day behavioral system of models.
 * Defines the sequence of models to be invoked and also handles dependencies between the models.
 * The models are specified by modelers in an external scripting language is invoked via this class.
 *
 * \note The only scripting language that is currently supported is Lua
 *
 * \author Harish Loganathan
 */
class PredaySystem {
private:
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;

	/**
	 * For each work tour, if the person has a usual work location, this function predicts whether the person goes to his usual location or some other location.
	 *
	 * @param personParam object containing person and household related variables
	 * @param firstOfMultiple indicator whether this tour is the first of multiple work tours
	 * @return true if the tour is to a usual work location. false otherwise.
	 */
	bool predictUsualWorkLocation(PersonParams& personParams, bool firstOfMultiple);

	/**
	 * Predicts the mode of travel for a tour.
	 * Executed for tours with usual location (usual work or education).
	 */
	void predictTourMode(Tour& tour);

	/**
	 * Predicts the mode and destination together for tours to unusual locations
	 */
	void predictTourModeDestination(Tour& tour);

	/**
	 * Predicts the time period that will be allotted for the primary activity of a tour.
	 */
	TimeWindowAvailability predictTourTimeOfDay(Tour& tour);

	/**
	 * Generates intermediate stops of types predicted by the day pattern model before and after the primary activity of a tour.
	 */
	void generateIntermediateStops(Tour& tour);

	/**
	 * Predicts the mode and destination together for stops.
	 */
	void predictStopModeDestination(Stop* stop, int origin);

	/**
	 * Predicts the arrival time for stops before the primary activity.
	 * Predicts the departure time for stops after the primary activity.
	 */
	void predictStopTimeOfDay(Stop* stop, bool isBeforePrimary);

	/**
	 * Calculates the arrival time for stops in the second half tour.
	 */
	void calculateArrivalTime(Stop* currentStop, Stop* prevStop);

	/**
	 * Calculates the departure time for stops in the first half tour.
	 */
	void calculateDepartureTime(Stop* currentStop, Stop* nextStop);

	/**
	 * Calculates the time to leave home for starting a tour.
	 */
	void calculateTourStartTime(Tour& tour);

	/**
	 * Calculates the time when the person reaches home at the end of the tour.
	 */
	void calculateTourEndTime(Tour& tour);

	/**
	 * constructs tour objects based on predicted number of tours. Puts the tour objects in tours deque.
	 */
	void constructTours();

	/**
	 * The parameters for a person is obtained from the population and set in personParams.
	 */
    PersonParams personParams;

    /**
     * Reference to map of zone_id -> ZoneParams
     * \note this map has 1092 elements
     */
    const ZoneMap& zoneMap;

    /**
     * Reference to Costs [origin zone, destination zone] -> CostParams*
     * \note these maps have (1092 zones * 1092 zones - 1092 (entries with same origin and destination is not available)) 1191372 elements
     */
    const CostMap& amCostMap;
    const CostMap& pmCostMap;
    const CostMap& opCostMap;

    /**
     * list of tours for this person
     */
    std::deque<Tour*> tours;

    /**
     * The predicted day pattern for the person indicating whether the person wants to make tours and stops of each type (Work, Education, Shopping, Others).
     */
    boost::unordered_map<std::string, bool> dayPattern;

    /**
     * The predicted number of tours for each type of tour - Work, Education, Shopping, Others.
     */
    boost::unordered_map<std::string, int> numTours;

    /**
     * Data access objects for mongo
     */
    boost::unordered_map<std::string, db::MongoDao*> mongoDao;

public:
	PredaySystem(PersonParams& personParams, const ZoneMap& zoneMap, const CostMap& amCostMap, const CostMap& pmCostMap, const CostMap& opCostMap);
	virtual ~PredaySystem();

	/**
	 * The sequence of models to be invoked for a person is coded in this function.
	 */
	void planDay();
};

} // end namespace medium
} // end namespace sim_mob
