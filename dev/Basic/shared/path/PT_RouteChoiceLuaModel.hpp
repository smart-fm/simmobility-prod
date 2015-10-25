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
/**
 * public transit route choice model
 * \author Zhang Huai Peng
 * \author Harish Loganathan
 */
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
	 * finds the best path for the given OD for public transit commuters
	 * @param origin		trip origin
	 * @param destination	trip destination
	 * @param odTrips		list of trip legs in pt path
	 * @return				true if route choice was successful; false otherwise
	 */
	bool getBestPT_Path(int origin, int destination, std::vector<sim_mob::OD_Trip>& odTrips);

	/**
	 * store chosen path in file
	 */
	void storeBestPT_Path();

private:
	PT_PathSet* publicTransitPathSet;
	std::vector<sim_mob::OD_Trip> odTripMapGen;
	soci::session* dbSession;
	std::string ptPathsetStoredProcName;

	/**
	 * load public transit pathset from database
	 * @param origin	trip origin
	 * @param dest		trip destination
	 * @return 			pathset retrieved from database
	 */
	PT_PathSet loadPT_PathSet(int origin, int dest);

	/**
     * Inherited from LuaModel
     */
	void mapClasses();

    /**
     * make public transit route choice from lua scripts.
	 * @param origin	trip origin
	 * @param dest		trip destination
     * @return 			the map from OD pair to pt trip
     */
	std::vector<sim_mob::OD_Trip> makePT_RouteChoice(const std::string& origin, const std::string& dest);

	/**
	 * get the size of current path set.
	 *
	 * @return the size of current choice set
	 */
	unsigned int getSizeOfChoiceSet();

};

}
