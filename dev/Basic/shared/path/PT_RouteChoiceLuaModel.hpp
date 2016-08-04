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
		return "PT pathset load failed exception";
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
	double getInVehicleTime(unsigned int index) const;

	/**
	 * interface function for Lua script to get total walking time
	 * @param index the index in public path set
	 * @return total walking time
	 */
	double getWalkTime(unsigned int index) const;

	/**
	 * interface functions for Lua script to get total waiting time
	 * @param index the index in public path set
	 * @return total waiting time
	 */
	double getWaitTime(unsigned int index) const;

	/**
	 * interface function for Lua script to get the size of path
	 * @param index the index in public path set
	 * @return the size of path
	 */
	double getPathSize(unsigned int index) const;

	/**
	 * interface function for Lua script to get total transfered time
	 * @param index the index in public path set
	 * @return total transfered time
	 */
	int getNumTxf(unsigned int index) const;

	/**
	 * interface function for Lua script to get total cost
	 * @param index the index in public path set
	 * @return total total cost
	 */
	double getCost(unsigned int index) const;

	/**
	 * interface function for Lua script to get path distance in PT vehicle (Bus or MRT)
	 * @param index the index in public path set
	 * @return total total cost
	 */
	double getPtDistanceKms(unsigned int index) const;

	/**
	 * interface function for lua script to get the PT modes in a path
	 * @param index the index of path in public path set
	 * @return 0 if path involves neither bus nor MRT travel;
	 *         1 if path involves only bus travel;
	 *         2 if path involves only MRT travel;
	 *         3 if path involves both bus and MRT travel
	 */
	int getModes(unsigned int index) const;

	/**
	 * finds the best path for the given OD for public transit commuters
	 * @param origin is trip origin
	 * @param destination is trip destination
	 * @param odTrips is list of trip legs in pt path
	 * @return true if route choice was successful; false otherwise
	 * @ptPathsetStoredProcName store procedure to fetch pathset
	 */
	bool getBestPT_Path(int origin, int destination, unsigned int startTime, std::vector<sim_mob::OD_Trip>& odTrips, std::string dbid, unsigned int start_time, const std::string& ptPathsetStoredProcName);
	/**
	 * fetches the public transit pathset for a given OD from database
	 * @param origin origin node id
	 * @param destination destination node id
	 * @param startTime time at which route choice is to be done
	 *
	 * @return public transit pathset for the supplied OD.
	 */
	// @ptPathsetStoredProcName is the store procedure to fetch pathsets.
	PT_PathSet fetchPathset(int origin, int destination, const DailyTime& startTime, const std::string& ptPathsetStoredProcName) const;
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
	 * @param curTime time at which routechoice is to be done
	 * @param pathSet output parameter for path set retrieved from database
	 * @param ptPathsetStoredProcName store procedure to fetch pathsets
	 * */
	void loadPT_PathSet(int origin, int dest, const DailyTime& curTime, PT_PathSet& pathSet, const std::string& ptPathsetStoredProcName) const;
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
	unsigned int getSizeOfChoiceSet() const;

};

}
