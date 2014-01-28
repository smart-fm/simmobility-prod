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
 * Invokes behavior models in a sequence as specified by the system of models.
 * Handles dependencies between models.
 * The models specified by modelers in an external scripting language are invoked via this class.
 *
 * \note The only scripting language that is currently supported is Lua
 *
 * \author Harish Loganathan
 */
class PredaySystem {
private:
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;
	typedef std::deque<Tour*> TourList;
	typedef std::deque<Stop*> StopList;

	/**
	 * For each work tour, if the person has a usual work location, this function predicts whether the person goes to his usual location or some other location.
	 *
	 * @param personParam object containing person and household related variables
	 * @param firstOfMultiple indicator whether this tour is the first of multiple work tours
	 * @return true if the tour is to a usual work location. false otherwise.
	 */
	bool predictUsualWorkLocation(bool firstOfMultiple);

	/**
	 * Predicts the mode of travel for a tour.
	 * Executed for tours with usual location (usual work or education).
	 *
	 * @param tour the tour for which the mode is to be predicted
	 */
	void predictTourMode(Tour* tour);

	/**
	 * Predicts the mode and destination together for tours to unusual locations
	 *
	 * @param tour the tour for which the mode and destination are to be predicted
	 */
	void predictTourModeDestination(Tour* tour);

	/**
	 * Predicts the time period that will be allotted for the primary activity of a tour.
	 *
	 * @param tour the tour for which the time of day is to be predicted
	 */
	TimeWindowAvailability predictTourTimeOfDay(Tour* tour);

	/**
	 * Generates intermediate stops of types predicted by the day pattern model before and after the primary activity of a tour.
	 *
	 * @param tour the tour for which stops are to be generated
	 */
	void generateIntermediateStops(Tour* tour);

	/**
	 * Predicts the mode and destination together for stops.
	 *
	 * @param stop the stop for which mode and destination are to be predicted
	 * @param origin the zone code for origin MTZ
	 */
	void predictStopModeDestination(Stop* stop, int origin);

	/**
	 * Predicts the arrival time for stops before the primary activity.
	 * Predicts the departure time for stops after the primary activity.
	 *
	 * @param stop the stop for which the time of day is to be predicted
	 * @param isBeforePrimary indicates whether stop is before the primary activity or after the primary activity of the tour
	 */
	void predictStopTimeOfDay(Stop* stop, bool isBeforePrimary);

	/**
	 * Calculates the arrival time for stops in the second half tour.
	 *
	 * @param currentStop the stop for which the arrival time is calculated
	 * @param prevStop the stop before currentStop
	 */
	void calculateArrivalTime(Stop* currentStop, Stop* prevStop);

	/**
	 * Calculates the departure time for stops in the first half tour.
	 *
	 * @param currentStop the stop for which the departure time is calculated
	 * @param nextStop the stop after currentStop
	 */
	void calculateDepartureTime(Stop* currentStop, Stop* nextStop);

	/**
	 * Calculates the time to leave home for starting a tour.
	 *
	 * @param tour the tour object for which the start time is to be calculated
	 */
	void calculateTourStartTime(Tour* tour);

	/**
	 * Calculates the time when the person reaches home at the end of the tour.
	 *
	 * @param tour the tour object for which the end time is to be calculated
	 */
	void calculateTourEndTime(Tour* tour);

	/**
	 * constructs tour objects based on predicted number of tours. Puts the tour objects in tours deque.
	 */
	void constructTours();

	/**
	 * inserts day pattern level information for a person
	 * This function will be called once after all predictions for every person in the population.
	 */
	void insertDayPattern();

	/**
	 * inserts tour level information for a person
	 * This function will be called once for every tour of every person in the population.
	 *
	 * @param tour an object containing information pertinent to a tour
	 * @param tourNumber the index of this tour among all tours of this person
	 */
	void insertTour(Tour* tour, int tourNumber);

	/**
	 * inserts tour level information for a person
	 * This function will be called once for every stop of every tour of every person in the population.
	 *
	 * @param stop an object containing information pertinent to a stop
	 * @param stopNumber the index of this stop among all stops of this tour
	 * @param tourNumber the index of the stop's parent tour among all tours of this person
	 */
	void insertStop(Stop* stop, int stopNumber, int tourNumber);

	/**
	 * Person specific parameters
	 */
	PersonParams& personParams;

    /**
     * Reference to map of zone_id -> ZoneParams
     * \note this map has 1092 elements
     */
    const ZoneMap& zoneMap;
    const boost::unordered_map<int,int>& zoneIdLookup;

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
    TourList tours;

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
	PredaySystem(PersonParams& personParams,
			const ZoneMap& zoneMap, const boost::unordered_map<int,int>& zoneIdLookup,
			const CostMap& amCostMap, const CostMap& pmCostMap, const CostMap& opCostMap,
			const boost::unordered_map<std::string, db::MongoDao*>& mongoDao);
	virtual ~PredaySystem();

	/**
	 * Invokes behavior models in a sequence as defined in the system of models
	 */
	void planDay();

	/**
	 * Writes the output of Preday to MongoDB
	 */
	void outputPredictionsToMongo();
};

} // end namespace medium
} // end namespace sim_mob
