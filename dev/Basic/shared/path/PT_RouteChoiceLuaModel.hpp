/*
 * PT_RouteChoiceLuaModel.hpp
 *
 *  Created on: 1 Apr, 2015
 *      Author: zhang
 */

#pragma once

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>

#include "entities/misc/PublicTransit.hpp"
#include "geospatial/aimsun/Loader.hpp"
#include "lua/LuaModel.hpp"
#include "Path.hpp"

namespace sim_mob
{
class PT_RouteChoiceLuaModel : public lua::LuaModel
{
public:
	PT_RouteChoiceLuaModel();
	virtual ~PT_RouteChoiceLuaModel();

	// interface functions for Lua script
	double getTotalInVehicleTime(unsigned int index);
	double getTotalWalkTime(unsigned int index);
	double getTotalWaitTime(unsigned int index);
	double getTotalPathSize(unsigned int index);
	int getTotalNumTxf(unsigned int index);
	double getTotalCost(unsigned int index);

	/**
	 *
	 */
	bool getBestPT_Path(const std::string& origin, const std::string& destination, std::vector<sim_mob::OD_Trip>& odTrips);
	void storeBestPT_Path();

private:
	PT_PathSet* publicTransitPathSet;
	std::vector<sim_mob::OD_Trip> odTripMapGen;
	soci::session* dbSession;
	std::string ptPathsetStoredProcName;

	/**
	 *
	 */
	PT_PathSet loadPT_PathSet(const std::string& original, const std::string& dest);

	/**
     * Inherited from LuaModel
     */
	void mapClasses();

    /**
     * make public transit route choice from lua scripts.
     *
     * return the map from OD pair to pt trip
     */
	std::vector<sim_mob::OD_Trip> makePT_RouteChoice(const std::string& original, const std::string& dest);

	/**
	 * get the size of current path set.
	 *
	 * return the size of current choice set
	 */
	unsigned int getSizeOfChoiceSet();

};

}
