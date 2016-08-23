/*
 * PT_RouteChoiceLuaModel.cpp
 *
 *  Created on: 1 Apr, 2015
 *      Author: zhang
 */

#include <cmath>
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "boost/foreach.hpp"
#include "boost/regex.hpp"
#include "boost/thread/mutex.hpp"
#include "Common.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/BusController.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "entities/PT_Statistics.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "logging/Log.hpp"
#include "lua/LuaLibrary.hpp"
#include "lua/third-party/luabridge/LuaBridge.h"
#include "lua/third-party/luabridge/RefCountedObject.h"
#include "PT_PathSetManager.hpp"
#include "PT_RouteChoiceLuaModel.hpp"
#include "SOCI_Converters.hpp"
#include "util/LangHelpers.hpp"

using namespace luabridge;

namespace
{
const double METERS_IN_UNIT_KM = 1000.0;
const std::string TWIN_BUS_STOP_PREFIX = "twin_";
}
namespace sim_mob
{

PT_RouteChoiceLuaModel::PT_RouteChoiceLuaModel() : publicTransitPathSet(nullptr), curStartTime()
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	dbSession = new soci::session(soci::postgresql, cfg.getDatabaseConnectionString(false));
	output.open("od_scenario.csv",std::ofstream::out);
}
PT_RouteChoiceLuaModel::~PT_RouteChoiceLuaModel()
{
	delete dbSession;
	output.close();
}

unsigned int PT_RouteChoiceLuaModel::getSizeOfChoiceSet() const
{
	unsigned int size = 0;
	if (publicTransitPathSet)
	{
		size = publicTransitPathSet->pathSet.size();
	}
	return size;
}

double PT_RouteChoiceLuaModel::getInVehicleTime(unsigned int index) const
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0) {
		std::set<PT_Path, cmp_path_vector>::const_iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getInVehicleTravelTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getWalkTime(unsigned int index) const
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0) {
		std::set<PT_Path, cmp_path_vector>::const_iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getWalkingTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getWaitTime(unsigned int index) const
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0) {
		std::set<PT_Path, cmp_path_vector>::const_iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getWaitingTimeSecs();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getPathSize(unsigned int index) const
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0)
	{
		std::set<PT_Path, cmp_path_vector>::const_iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getPathSize();
	}
	return ret;
}

int PT_RouteChoiceLuaModel::getNumTxf(unsigned int index) const
{
	int ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0)
	{
		std::set<PT_Path, cmp_path_vector>::const_iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getNumTransfers();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getCost(unsigned int index) const
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0)
	{
		std::set<PT_Path, cmp_path_vector>::const_iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getPathCost();
	}
	return ret;
}

double PT_RouteChoiceLuaModel::getPtDistanceKms(unsigned int index) const
{
	double ret = 0.0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if (publicTransitPathSet && index <= sizeOfChoiceSet && index > 0)
	{
		std::set<PT_Path, cmp_path_vector>::const_iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getPtDistanceKms();
	}
	return ret;
}

int PT_RouteChoiceLuaModel::getModes(unsigned int index) const
{
	int ret = 0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if(publicTransitPathSet && index <= sizeOfChoiceSet && index > 0)
	{
		std::set<PT_Path, cmp_path_vector>::const_iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		ret = it->getPathModesType();
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

	if (publicTransitPathSet)
	{
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
		std::advance(it, index - 1);
		const std::vector<PT_NetworkEdge>& pathEdges = it->getPathEdges();
		for (std::vector<PT_NetworkEdge>::const_iterator itEdge = pathEdges.begin(); itEdge != pathEdges.end(); itEdge++) {
			sim_mob::OD_Trip trip;
			trip.startStop = itEdge->getStartStop();
			trip.sType = PT_NetworkCreater::getInstance().getVertexTypeFromStopId(trip.startStop);
			if (trip.startStop.find("N_") != std::string::npos)
			{
				trip.startStop = trip.startStop.substr(2);
			}
			trip.endStop = itEdge->getEndStop();
			trip.eType = PT_NetworkCreater::getInstance().getVertexTypeFromStopId(trip.endStop);
			if (trip.endStop.find("N_") != std::string::npos)
			{
				trip.endStop = trip.endStop.substr(2);
			}
			trip.tType = itEdge->getType();
			trip.tTypeStr = itEdge->getTypeStr();
			trip.serviceLines = itEdge->getServiceLines();
			trip.originNode = origin;
			trip.destNode = destination;
			trip.scenario = it->getScenario();
			trip.travelTime = itEdge->getTransitTimeSecs();
			trip.walkTime = itEdge->getWalkTimeSecs();
			trip.id = itEdge->getEdgeId();
			trip.pathset = pathSetId;
			trip.serviceLine = itEdge->getServiceLine();
			odTrips.push_back(trip);
		}
	}

	return odTrips;
}

PT_PathSet PT_RouteChoiceLuaModel::fetchPathset(int origin, int destination, const DailyTime& startTime, const std::string& ptPathsetStoredProcName) const
{
	PT_PathSet pathSet;
	loadPT_PathSet(origin, destination, startTime, pathSet,ptPathsetStoredProcName);
	return pathSet;
}

bool PT_RouteChoiceLuaModel::getBestPT_Path(int origin, int dest, unsigned int startTime, std::vector<sim_mob::OD_Trip>& odTrips, std::string dbid, unsigned int start_time, const std::string& ptPathsetStoredProcName)
{
	bool ret = false;
	PT_PathSet pathSet;
	curStartTime = DailyTime(startTime);
	loadPT_PathSet(origin, dest, DailyTime(startTime), pathSet,ptPathsetStoredProcName);
	if (pathSet.pathSet.empty())
	{
		sim_mob::BasicLogger& ptPathsetLogger  = sim_mob::Logger::log("pt_pathset_failed.csv");
		ptPathsetLogger << origin << "," << dest << ","<<curStartTime.getStrRepr() << std::endl;
		throw PT_PathsetLoadException(origin, dest);
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

void PT_RouteChoiceLuaModel::printScenarioAndOD(const std::vector<sim_mob::OD_Trip>& odTrips, std::string dbid, unsigned int startTime)
{
	if (odTrips.empty())
	{
		return;
	}
	output << odTrips.front().originNode << " , " << odTrips.front().destNode << " , " << odTrips.front().scenario << " , " << dbid << ", " << startTime
			<< std::endl;
}

void PT_RouteChoiceLuaModel::storeBestPT_Path()
{
	std::ofstream outputFile("od_to_trips.csv");
	if (outputFile.is_open())
	{
		std::vector<sim_mob::OD_Trip>::iterator odIt = odTripMapGen.begin();
		for (; odIt != odTripMapGen.end(); odIt++)
		{
			outputFile << odIt->startStop << ",";
			outputFile << odIt->endStop << ",";
			outputFile << odIt->sType << ",";
			outputFile << odIt->eType << ",";
			outputFile << odIt->tTypeStr << ",";
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
			.addFunction("total_in_vehicle_time",&PT_RouteChoiceLuaModel::getInVehicleTime)
			.addFunction("total_walk_time",&PT_RouteChoiceLuaModel::getWalkTime)
			.addFunction("total_wait_time",&PT_RouteChoiceLuaModel::getWaitTime)
			.addFunction("total_path_size",&PT_RouteChoiceLuaModel::getPathSize)
			.addFunction("total_no_txf", &PT_RouteChoiceLuaModel::getNumTxf)
			.addFunction("total_cost", &PT_RouteChoiceLuaModel::getCost)
			.addFunction("pt_distance_km", &PT_RouteChoiceLuaModel::getPtDistanceKms)
			.addFunction("path_pt_modes", &PT_RouteChoiceLuaModel::getModes)
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

void PT_RouteChoiceLuaModel::loadPT_PathSet(int origin, int dest, const DailyTime& curTime, PT_PathSet& pathSet, const std::string& ptPathsetStoredProcName) const
{
	const BusController* busController = BusController::GetInstance();
	const TravelTimeManager* ttMgr = TravelTimeManager::getInstance();
	const PT_Statistics* ptStats = PT_Statistics::getInstance();

	std::vector<sim_mob::PT_Path> paths;
	loadPT_PathsetFromDB(*dbSession, ptPathsetStoredProcName, origin, dest, paths);
	for(auto& path : paths)
	{
		std::vector<PT_NetworkEdge> pathEdges = path.getPathEdges();

		//compute and set path travel time
		double pathTravelTime = 0.0;
		double pathWaitingTime = 0.0;
		double pathInVehicleTravelTime = 0.0;
		double pathPtDistanceInMts = 0.0;
		DailyTime nextStartTime = curTime;
		bool invalidPath = false;
		for(PT_NetworkEdge& edge : pathEdges)
		{
			if(edge.getStartStop() == edge.getEndStop())
			{
				invalidPath = true;
				break;
			}
			switch(edge.getType())
			{
			case sim_mob::BUS_EDGE:
			{
				if(!busController)
				{
					invalidPath = true;
					break;
				}

				double edgeTravelTime = 0.0;
				const std::string& busLineIds = edge.getServiceLines();
				std::vector<std::string> lines;
				boost::split(lines, busLineIds, boost::is_any_of("/"));

				if(!busController->isBuslineAvailable(lines, nextStartTime))
				{
					invalidPath = true;
					break;
				}

				const std::vector<const Link*>& busRouteLinks = busController->getLinkRoute(lines.front());
				const std::vector<const BusStop*>& busRouteStops = busController->getStops(lines.front());

				const BusStop* originStop = BusStop::findBusStop(edge.getStartStop());
				if(!originStop)
				{
					throw std::runtime_error("invalid origin stop in path edge: " + edge.getStartStop());
				}
				if(originStop->getTerminusType() == sim_mob::SINK_TERMINUS)
				{
					originStop = originStop->getTwinStop(); // origin stop should not be a sink terminus
				}

				std::string originStopCode = originStop->getStopCode();
				if(originStopCode.find(TWIN_BUS_STOP_PREFIX) != std::string::npos) {	originStopCode = originStop->getTwinStop()->getStopCode(); }

				double waitingTime = ptStats->getWaitingTime((nextStartTime.getValue()/1000), originStopCode, lines.front());
				if(waitingTime > 0)
				{
					edgeTravelTime = edgeTravelTime + waitingTime;
					nextStartTime = DailyTime(nextStartTime.getValue() + std::floor(waitingTime*1000));
				}
				else
				{
					waitingTime = edge.getWaitTimeSecs();
					edgeTravelTime = edgeTravelTime + waitingTime;
					nextStartTime = DailyTime(nextStartTime.getValue() + std::floor(waitingTime*1000));
				}

				std::vector<const BusStop*>::const_iterator nextStopIt = std::find(busRouteStops.begin(), busRouteStops.end(), originStop);
				if(nextStopIt == busRouteStops.end())
				{
					throw std::runtime_error("origin stop not found in bus route stops");
				}
				nextStopIt++; //nextStop points to first stop after origin

				const BusStop* destinStop = BusStop::findBusStop(edge.getEndStop());
				if(!destinStop)
				{
					throw std::runtime_error("invalid origin stop in path edge: " + edge.getEndStop());
				}
				if(destinStop->getTerminusType() == sim_mob::SOURCE_TERMINUS)
				{
					destinStop = destinStop->getTwinStop(); // destination stop should not be a source terminus
				}

				const Link* originLink = originStop->getParentSegment()->getParentLink();
				const Link* destinLink = destinStop->getParentSegment()->getParentLink();
				std::vector<const Link*>::const_iterator lnkIt = std::find(busRouteLinks.begin(), busRouteLinks.end(), originLink);
				if(lnkIt == busRouteLinks.end())
				{
					throw std::runtime_error("origin link not found in bus route");
				}
				std::vector<const Link*>::const_iterator nextLnkIt = lnkIt+1;
				if(destinLink!=originLink && nextLnkIt == busRouteLinks.end())
				{	//we have hit the end before encountering the destination link
					throw std::runtime_error("destination stop's link not found in bus route");
				}

				const Link* currentLink = nullptr;
				const Link* nextLink = nullptr;
				const BusStop* nextStop = nullptr;
				double tt = 0.0;
				bool isBeforeDestStop = true;
				for(; lnkIt != busRouteLinks.end(); lnkIt++, nextLnkIt++)
				{
					currentLink = *lnkIt;
					if(nextLnkIt == busRouteLinks.end()) { nextLink = nullptr; }
					else { nextLink = *nextLnkIt; }

					tt = ttMgr->getLinkTT(currentLink, nextStartTime, nextLink);

					nextStop = *nextStopIt;
					while(isBeforeDestStop && nextStop && nextStop->getParentSegment()->getParentLink() == currentLink)
					{
						isBeforeDestStop = !((nextStop == destinStop) || (nextStop->getTwinStop() == destinStop));
						std::string nextStopCode = nextStop->getStopCode();
						if(nextStopCode.find(TWIN_BUS_STOP_PREFIX) != std::string::npos) { nextStop->getTwinStop()->getStopCode(); }
						double dwellTime = ptStats->getDwellTime((nextStartTime.getValue()/1000), nextStopCode, lines.front());
						if(dwellTime > 0)
						{
							tt = tt + dwellTime;
						}
						nextStopIt++;
						if(nextStopIt == busRouteStops.end()) { nextStop = nullptr; }
						else { nextStop = *nextStopIt; }
					}

					edgeTravelTime = edgeTravelTime + tt;
					pathInVehicleTravelTime = pathInVehicleTravelTime + tt;
					pathPtDistanceInMts = pathPtDistanceInMts + currentLink->getLength();
					nextStartTime = DailyTime(nextStartTime.getValue() + std::floor(tt*1000));

					if(!isBeforeDestStop)
					{
						break;
					}
				}
				if(lnkIt == busRouteLinks.end())
				{	//we have hit the end before encountering the destination stop
					throw std::runtime_error("destination stop's link not found in bus route");
				}

				edge.setLinkTravelTimeSecs(edgeTravelTime);
				pathTravelTime = pathTravelTime + edgeTravelTime;
				pathWaitingTime = pathWaitingTime + waitingTime;
				break;
			}
			case sim_mob::TRAIN_EDGE:
			{
				double edgeTravelTime =  edge.getLinkTravelTimeSecs() + edge.getWalkTimeSecs() + edge.getWaitTimeSecs();
				edge.setLinkTravelTimeSecs(edgeTravelTime);
				pathTravelTime = pathTravelTime + edgeTravelTime;
				pathWaitingTime = pathWaitingTime + edge.getWaitTimeSecs();
				pathInVehicleTravelTime = pathInVehicleTravelTime + edge.getLinkTravelTimeSecs();
				pathPtDistanceInMts = pathPtDistanceInMts + (edge.getDistKms() * 1000);
				nextStartTime = DailyTime(nextStartTime.getValue() + std::floor(edgeTravelTime*1000));
				break;
			}
			case sim_mob::WALK_EDGE:
			{
				double edgeTravelTime = edge.getLinkTravelTimeSecs() + edge.getWaitTimeSecs(); //same as walk time for walk edges
				edge.setLinkTravelTimeSecs(edgeTravelTime);
				pathTravelTime = pathTravelTime + edgeTravelTime;
				nextStartTime = DailyTime(nextStartTime.getValue() + std::floor(edgeTravelTime*1000));
				break;
			}
			default:
			{
				throw std::runtime_error("Unknown PT edge type found in path");
			}
			}
		}
		if(invalidPath) { continue; }
		path.setPathEdges(pathEdges);
		path.setPathTravelTime(pathTravelTime);
		path.setWaitingTimeSecs(pathWaitingTime);
		path.setInVehicleTravelTimeSecs(pathInVehicleTravelTime);
		path.setPtDistanceKms(pathPtDistanceInMts/METERS_IN_UNIT_KM);
		pathSet.pathSet.insert(path);
	}
	pathSet.computeAndSetPathSize();
}

}
