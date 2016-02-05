#pragma once

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include "entities/misc/PublicTransit.hpp"
#include "lua/LuaModel.hpp"
#include "Path.hpp"
#include "soci/soci.h"

namespace sim_mob
{
/**
 * public transit route choice model
 * \author Zhang Huai Peng
 * \author Harish Loganathan
 */
class PT_RouteChoiceLuaModel: public lua::LuaModel
{
public:
	PT_RouteChoiceLuaModel();
	virtual ~PT_RouteChoiceLuaModel();

	/**
	 * interface functions for Lua script to get total time in vehicle
	 * @param index the index in public path set
	 * @return total time in vehicle
	 */
	double getTotalInVehicleTime(unsigned int index);

	/**
	 * interface functions for Lua script to get total walking time
	 * @param index the index in public path set
	 * @return total walking time
	 */
	double getTotalWalkTime(unsigned int index);

	/**
	 * interface functions for Lua script to get total waiting time
	 * @param index the index in public path set
	 * @return total waiting time
	 */
	double getTotalWaitTime(unsigned int index);

	/**
	 * interface functions for Lua script to get the size of path
	 * @param index the index in public path set
	 * @return the size of path
	 */
	double getTotalPathSize(unsigned int index);

	/**
	 * interface functions for Lua script to get total transfered time
	 * @param index the index in public path set
	 * @return total transfered time
	 */
	int getTotalNumTxf(unsigned int index);

	/**
	 * interface functions for Lua script to get total cost
	 * @param index the index in public path set
	 * @return total total cost
	 */
	double getTotalCost(unsigned int index);

	/**
	 * finds the best path for the given OD for public transit commuters
	 * @param origin is trip origin
	 * @param destination is trip destination
	 * @param odTrips is list of trip legs in pt path
	 * @return true if route choice was successful; false otherwise
	 */
	bool getBestPT_Path(int origin, int destination, unsigned int startTime, std::vector<sim_mob::OD_Trip>& odTrips);

	/**
	 * store chosen path in file
	 */
	void storeBestPT_Path();

private:
	/**public path set for a given O-D pair*/
	PT_PathSet* publicTransitPathSet;

	/**the concrete trip from public route choice*/
	std::vector<sim_mob::OD_Trip> odTripMapGen;

	/**database session for loading public path set*/
	soci::session* dbSession;

	/**the name of stored-procedure for loading public path set*/
	std::string ptPathsetStoredProcName;

	/**start time for current trip*/
	unsigned curStartTime;

	/**
	 * load public transit path set from database
	 * @param origin is trip origin
	 * @param dest is trip destination
	 * @return path set retrieved from database
	 */
	PT_PathSet loadPT_PathSet(int origin, int dest);

	/**
	 * Inherited from LuaModel
	 */
	void mapClasses();

	/**
	 * make public transit route choice from lua scripts.
	 * @param origin is	trip origin
	 * @param dest	is	trip destination
	 * @return the map from OD pair to public transit trip
	 */
	std::vector<sim_mob::OD_Trip> makePT_RouteChoice(const std::string& origin, const std::string& dest);

	/**
	 * get the size of current path set.
	 * @return the size of current choice set
	 */
	unsigned int getSizeOfChoiceSet();

};

}
