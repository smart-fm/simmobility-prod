/*
 * PT_RouteChoiceLuaModel.hpp
 *
 *  Created on: 1 Apr, 2015
 *      Author: zhang
 */

#pragma once

#include "lua/LuaModel.hpp"
#include "Path.hpp"
#include "util/Utils.hpp"
#include "Common.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "entities/misc/PublicTransit.hpp"

namespace sim_mob{

class PT_RouteChoiceLuaModel : public lua::LuaModel  {
public:
	PT_RouteChoiceLuaModel();
	virtual ~PT_RouteChoiceLuaModel();

    /**
      * retrieve a singleton object
      * @return a pointer to PT_RouteChoiceModel .
      */
	static PT_RouteChoiceLuaModel* Instance();

	/**
	 * interfaces for Lua script
	 * @param index is the index in the choice set
	 * @return the corresponding property value
	 */
	double total_in_vehicle_time(unsigned int index);
	double total_walk_time(unsigned int index);
	double total_wait_time(unsigned int index);
	double total_path_size(unsigned int index);
	int total_no_txf(unsigned int index);
	double total_cost(unsigned int index);

	bool GetBestPT_Path(const std::string& original, const std::string& dest, std::vector<sim_mob::OD_Trip>& odTrips);

private:
	PT_PathSet* publicTransitPathSet;
	static PT_RouteChoiceLuaModel* instance;
	std::map<boost::thread::id, boost::shared_ptr<soci::session > > cnnRepo;
	boost::shared_mutex cnnRepoMutex;

private:
	/**
	 * get the database session used for this thread
	 */
	const boost::shared_ptr<soci::session> & getSession();

	PT_PathSet LoadPT_PathSet(const std::string& original, const std::string& dest);
    /**
     * Inherited from LuaModel
     */
	void mapClasses();

    /**
     * make public transit route choice from lua scripts.
     *
     * return the map from OD pair to pt trip
     */
	std::vector<sim_mob::OD_Trip> MakePT_RouteChoice(const std::string& original, const std::string& dest);

	void SetPathSet(PT_PathSet* set){
		publicTransitPathSet = set;
	}

	/**
	 * get the size of current path set.
	 *
	 * return the size of current choice set
	 */
	unsigned int GetSizeOfChoiceSet();
};

}
