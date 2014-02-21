//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * PredayManager.hpp
 *
 *  Created on: Nov 18, 2013
 *      Author: Harish Loganathan
 */

#pragma once
#include <boost/unordered_map.hpp>
#include "params/PersonParams.hpp"
#include "params/ZoneCostParams.hpp"
#include "database/DB_Connection.hpp"

namespace sim_mob {
namespace medium {
class PredayManager {
public:

	virtual ~PredayManager();

	/**
	 * Gets person data from the database and stores corresponding PersonParam pointers in personList.
	 *
	 * @param dbType type of backend where the population data is available
	 */
	void loadPersons(db::BackendType dbType);

	/**
	 * Gets details of all mtz zones
	 *
	 * @param dbType type of backend where the zone data is available
	 */
	void loadZones(db::BackendType dbType);

	/**
	 * loads the AM, PM and off peak costs data
	 *
	 * @param dbType type of backend where the cost data is available
	 */
	void loadCosts(db::BackendType dbType);

	/**
	 * Distributes persons to different threads and starts the threads which process the persons
	 *
	 * @param numWorkers number of threads to create for processing the person list
	 */
	void distributeAndProcessPersons(unsigned numWorkers = 1);

private:
	typedef std::vector<PersonParams*> PersonList;
	typedef boost::unordered_map<int, ZoneParams*> ZoneMap;
	typedef boost::unordered_map<int, boost::unordered_map<int, CostParams*> > CostMap;

	/**
	 * Threaded function loop.
	 * Loops through all elements in personList within the specified range and
	 * invokes the Preday system of models for each of them.
	 *
	 * @param first personList iterator corresponding to the first person to be
	 * 				processed
	 * @param last personList iterator corresponding to the person after the
	 * 				last person to be processed
	 */
	void processPersons(PersonList::iterator first, PersonList::iterator last);

	PersonList personList;

    /**
     * map of zone_id -> ZoneParams
     * \note this map has 1092 elements
     */
    ZoneMap zoneMap;
    boost::unordered_map<int,int> zoneIdLookup;

    /**
     * Map of AM, PM and Off peak Costs [origin zone, destination zone] -> CostParams*
     * \note these maps have (1092 zones * 1092 zones - 1092 (entries with same origin and destination is not available)) 1191372 elements
     */
    CostMap amCostMap;
    CostMap pmCostMap;
    CostMap opCostMap;

};
} //end namespace medium
} //end namespace sim_mob



