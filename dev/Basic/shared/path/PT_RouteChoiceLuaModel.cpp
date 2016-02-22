/*
 * PT_RouteChoiceLuaModel.cpp
 *
 *  Created on: 1 Apr, 2015
 *      Author: zhang
 */

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"
#include "boost/regex.hpp"
#include "boost/thread/mutex.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "logging/Log.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "PT_EdgeTravelTime.hpp"
#include "PT_PathSetManager.hpp"
#include "PT_RouteChoiceLuaModel.hpp"
#include "SOCI_Converters.hpp"
#include "util/LangHelpers.hpp"

using namespace luabridge;

namespace sim_mob
{

PT_RouteChoiceLuaModel::PT_RouteChoiceLuaModel() : publicTransitPathSet(nullptr), curStartTime(0)
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	ptPathsetStoredProcName = cfg.getDatabaseProcMappings().procedureMappings["pt_pathset"];
	if (ptPathsetStoredProcName.empty())
	{
		throw std::runtime_error("stored procedure for \"pt_pathset\" is empty or not specified in config file");
	}
	dbSession = new soci::session(soci::postgresql, cfg.getDatabaseConnectionString(false));
}
PT_RouteChoiceLuaModel::~PT_RouteChoiceLuaModel()
{
	delete dbSession;
}

unsigned int PT_RouteChoiceLuaModel::getSizeOfChoiceSet()
{
	unsigned int size = 0;
	if (publicTransitPathSet) {
		size = publicTransitPathSet->pathSet.size();
	}
	return size;
}

double PT_RouteChoiceLuaModel::getTotalInVehicleTime(unsigned int index)
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0) {
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getTotalInVehicleTravelTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getTotalWalkTime(unsigned int index)
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0) {
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getTotalWalkingTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getTotalWaitTime(unsigned int index)
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0) {
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getTotalWaitingTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getTotalPathSize(unsigned int index)
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0)
	{
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getPathSize();
	}
	return ret;
}

int PT_RouteChoiceLuaModel::getTotalNumTxf(unsigned int index)
{
	int ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0)
	{
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getTotalNumberOfTransfers();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getTotalCost(unsigned int index)
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0) {
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getTotalCost();
	}
	return ret;
}

std::vector<sim_mob::OD_Trip> PT_RouteChoiceLuaModel::makePT_RouteChoice(const std::string& origin, const std::string& destination)
{
	std::vector<sim_mob::OD_Trip> odTrips;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	std::string pathSetId = "N_" + origin + "_" + "N_" + destination;
	LuaRef funcRef = getGlobal(state.get(), "choose_PT_path");
	LuaRef retVal = funcRef(this, sizeOfChoiceSet);
	int index = -1;
	if (retVal.isNumber()) {
		index = retVal.cast<int>();
	}

	if (index > sizeOfChoiceSet || index <= 0) {
		std::stringstream errStrm;
		errStrm << "invalid path index (" << index
				<< ") returned from PT route choice for OD " << pathSetId
				<< " with " << sizeOfChoiceSet << "path choices" << std::endl;
		throw std::runtime_error(errStrm.str());
	}

	if (publicTransitPathSet) {
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		const std::vector<PT_NetworkEdge>& pathEdges = it->getPathEdges();
		for (std::vector<PT_NetworkEdge>::const_iterator itEdge = pathEdges.begin(); itEdge != pathEdges.end(); itEdge++) {
			sim_mob::OD_Trip trip;
			trip.startStop = itEdge->getStartStop();
			trip.sType = PT_Network::getInstance().getVertexTypeFromStopId(trip.startStop);
			if (trip.startStop.find("N_") != std::string::npos) {
				trip.startStop = trip.startStop.substr(2);
			}
			trip.endStop = itEdge->getEndStop();
			trip.eType = PT_Network::getInstance().getVertexTypeFromStopId(trip.endStop);
			if (trip.endStop.find("N_") != std::string::npos) {
				trip.endStop = trip.endStop.substr(2);
			}
			trip.tType = itEdge->getType();
			trip.serviceLines = itEdge->getServiceLines();
			trip.originNode = origin;
			trip.destNode = destination;
			trip.travelTime = itEdge->getTransitTimeSecs();
			trip.walkTime = itEdge->getWalkTimeSecs();
			trip.id = itEdge->getEdgeId();
			trip.pathset = pathSetId;
			odTrips.push_back(trip);
		}
	}

	return odTrips;
}

bool PT_RouteChoiceLuaModel::getBestPT_Path(int origin, int dest, unsigned int startTime, std::vector<sim_mob::OD_Trip>& odTrips)
{
	bool ret = false;
	PT_PathSet pathSet;
	curStartTime = startTime;
	loadPT_PathSet(origin, dest, pathSet);
	if (pathSet.pathSet.empty())
	{
		Print() << "[PT pathset]load pathset failed:[" << origin << "]:[" << dest << "]" << std::endl;
	}
	else
	{
		std::string originId = boost::lexical_cast < std::string > (origin);
		std::string destId = boost::lexical_cast < std::string > (dest);
		publicTransitPathSet = &pathSet;
		odTrips = makePT_RouteChoice(originId, destId);
		ret = true;
	}
	return ret;
}

void PT_RouteChoiceLuaModel::storeBestPT_Path()
{
	std::ofstream outputFile("od_to_trips.csv");
	if (outputFile.is_open()) {
		std::vector<sim_mob::OD_Trip>::iterator odIt = odTripMapGen.begin();
		for (; odIt != odTripMapGen.end(); odIt++) {
			outputFile << odIt->startStop << ",";
			outputFile << odIt->endStop << ",";
			outputFile << odIt->sType << ",";
			outputFile << odIt->eType << ",";
			outputFile << odIt->tType << ",";
			outputFile << odIt->serviceLines << ",";
			outputFile << odIt->pathset << ",";
			outputFile << odIt->id << ",";
			outputFile << odIt->originNode << ",";
			outputFile << odIt->destNode << ",";
			outputFile << odIt->travelTime << std::endl;
		}
		outputFile.close();
	}
}

void PT_RouteChoiceLuaModel::mapClasses()
{
	getGlobalNamespace(state.get()).beginClass <PT_RouteChoiceLuaModel> ("PT_RouteChoiceLuaModel")
			.addFunction("total_in_vehicle_time",&PT_RouteChoiceLuaModel::getTotalInVehicleTime)
			.addFunction("total_walk_time",&PT_RouteChoiceLuaModel::getTotalWalkTime)
			.addFunction("total_wait_time",&PT_RouteChoiceLuaModel::getTotalWaitTime)
			.addFunction("total_path_size",&PT_RouteChoiceLuaModel::getTotalPathSize)
			.addFunction("total_no_txf", &PT_RouteChoiceLuaModel::getTotalNumTxf)
			.addFunction("total_cost", &PT_RouteChoiceLuaModel::getTotalCost)
			.endClass();
}

void loadPT_PathsetFromDB(soci::session& sql, const std::string& funcName, int originNode, int destNode, std::vector<sim_mob::PT_Path>& paths)
{
	soci::rowset<sim_mob::PT_Path> rs =
			(sql.prepare << std::string("select * from ") + funcName + "(:o_node,:d_node)", soci::use(originNode), soci::use(destNode));
	for (soci::rowset<sim_mob::PT_Path>::const_iterator it = rs.begin(); it != rs.end(); ++it)
	{
		paths.push_back(*it);
	}
}

void PT_RouteChoiceLuaModel::loadPT_PathSet(int origin, int dest, PT_PathSet& pathSet)
{
	std::vector<sim_mob::PT_Path> paths;
	loadPT_PathsetFromDB(*dbSession, ptPathsetStoredProcName, origin, dest, paths);
	const PT_EdgeTravelTime* ptEdgeTT = PT_EdgeTravelTime::getInstance();
	for(auto& path : paths)
	{
		std::vector<PT_NetworkEdge> pathEdges = path.getPathEdges();
		unsigned int startTime = curStartTime;
		double pathTravelTime = 0;
		for(auto& pathEdge : pathEdges)
		{
			int edgeId = pathEdge.getEdgeId();
			double waitTime = 0;
			double walkTime = 0;
			double dayTransitTime = 0;
			double edgeTravelTime = 0;
			bool ttFetched = ptEdgeTT->getEdgeTravelTime(edgeId, startTime, waitTime, walkTime, dayTransitTime, edgeTravelTime);
			if(ttFetched)
			{
				pathEdge.setWaitTimeSecs(waitTime);
				pathEdge.setWalkTimeSecs(walkTime);
				pathEdge.setDayTransitTimeSecs(dayTransitTime);
				pathEdge.setTransitTimeSecs(dayTransitTime);
				pathEdge.setLinkTravelTimeSecs(edgeTravelTime);
			}
			pathTravelTime = pathTravelTime + pathEdge.getLinkTravelTimeSecs();
			startTime = startTime + pathEdge.getLinkTravelTimeSecs()*1000;
		}
		path.setPathEdges(pathEdges);
		path.setPathTravelTime(pathTravelTime);
		pathSet.pathSet.insert(path);
		pathSet.computeAndSetPathSize();
	}
}

}
