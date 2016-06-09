#pragma once

#include <boost/shared_ptr.hpp>
#include <map>
#include <vector>
#include <fstream>
#include "entities/misc/PublicTransit.hpp"
#include "lua/LuaModel.hpp"
#include "Path.hpp"
#include "soci/soci.h"
#include "util/DailyTime.hpp"

namespace sim_mob
{
/**
 * the exception for pt pathset loading
 * \author Zhang Huai Peng
 */
class PT_PathsetLoadException: public std::exception {
public:
	int originNode;
	int destNode;
	PT_PathsetLoadException(int sNode, int dNode):
		originNode(sNode),destNode(dNode){}
	virtual const char* what() const throw () {
		return "the exception of pt pathset loading";
	}
};
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
	 * interface function for Lua script to get total time in vehicle
	 * @param index the index in public path set
	 * @return total time in vehicle
	 */
	double getTotalInVehicleTime(unsigned int index);

	/**
	 * interface function for Lua script to get total walking time
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
	 * interface function for Lua script to get the size of path
	 * @param index the index in public path set
	 * @return the size of path
	 */
	double getTotalPathSize(unsigned int index);

	/**
	 * interface function for Lua script to get total transfered time
	 * @param index the index in public path set
	 * @return total transfered time
	 */
	int getTotalNumTxf(unsigned int index);

	/**
	 * interface function for Lua script to get total cost
	 * @param index the index in public path set
	 * @return total total cost
	 */
	double getTotalCost(unsigned int index);

	/**
	 * interface function for lua script to get the PT modes in a path
	 * @param index the index of path in public path set
	 * @return 0 if path involves neither bus nor MRT travel;
	 *         1 if path involves only bus travel;
	 *         2 if path involves only MRT travel;
	 *         3 if path involves both bus and MRT travel
	 */
	int getModes(unsigned int index);

	/**
	 * finds the best path for the given OD for public transit commuters
	 * @param origin is trip origin
	 * @param destination is trip destination
	 * @param odTrips is list of trip legs in pt path
	 * @return true if route choice was successful; false otherwise
	 */
	bool getBestPT_Path(int origin, int destination, unsigned int startTime, std::vector<sim_mob::OD_Trip>& odTrips, std::string dbid, unsigned int start_time, const std::string& ptPathsetStoredProcName);
	/**
	 * store chosen path in file
	 */
	void storeBestPT_Path();

	void printScenarioAndOD(const std::vector<sim_mob::OD_Trip>& odTrips, std::string dbid, unsigned int startTime);

private:
	/**public path set for a given O-D pair*/
	PT_PathSet* publicTransitPathSet;

	/**the concrete trip from public route choice*/
	std::vector<sim_mob::OD_Trip> odTripMapGen;

	/**database session for loading public path set*/
	soci::session* dbSession;

	/**start time for current trip*/
	DailyTime curStartTime;

	std::ofstream output;

	/**
	 * load public transit path set from database
	 * @param origin is trip origin
	 * @param dest is trip destination
	 * @param pathSet output parameter for path set retrieved from database
	 */
	void loadPT_PathSet(int origin, int dest, PT_PathSet& pathSet, const std::string& ptPathsetStoredProcName);
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
