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

int PT_RouteChoiceLuaModel::getModes(unsigned int index)
{
	int ret = 0;
	unsigned int sizeOfChoiceSet = getSizeOfChoiceSet();
	if(publicTransitPathSet && index <= sizeOfChoiceSet && index > 0)
	{
		std::set<PT_Path, cmp_path_vector>::iterator it = publicTransitPathSet->pathSet.begin();
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

bool PT_RouteChoiceLuaModel::getBestPT_Path(int origin, int dest, unsigned int startTime, std::vector<sim_mob::OD_Trip>& odTrips, std::string dbid, unsigned int start_time, const std::string& ptPathsetStoredProcName)
{
	bool ret = false;
	PT_PathSet pathSet;
	curStartTime = DailyTime(startTime);
	loadPT_PathSet(origin, dest, pathSet,ptPathsetStoredProcName);
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
		//printScenarioAndOD(odTrips, dbid, start_time);
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
	if (outputFile.is_open()) {
		std::vector<sim_mob::OD_Trip>::iterator odIt = odTripMapGen.begin();
		for (; odIt != odTripMapGen.end(); odIt++) {
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
			.addFunction("total_in_vehicle_time",&PT_RouteChoiceLuaModel::getTotalInVehicleTime)
			.addFunction("total_walk_time",&PT_RouteChoiceLuaModel::getTotalWalkTime)
			.addFunction("total_wait_time",&PT_RouteChoiceLuaModel::getTotalWaitTime)
			.addFunction("total_path_size",&PT_RouteChoiceLuaModel::getTotalPathSize)
			.addFunction("total_no_txf", &PT_RouteChoiceLuaModel::getTotalNumTxf)
			.addFunction("total_cost", &PT_RouteChoiceLuaModel::getTotalCost)
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

void PT_RouteChoiceLuaModel::loadPT_PathSet(int origin, int dest, PT_PathSet& pathSet, const std::string& ptPathsetStoredProcName)
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
		DailyTime nextStartTime = curStartTime;
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

				double waitingTime = ptStats->getWaitingTime((nextStartTime.getValue()/1000), originStop->getStopCode(), lines.front());
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
						double dwellTime = ptStats->getDwellTime((nextStartTime.getValue()/1000), nextStop->getStopCode(), lines.front());
						if(dwellTime > 0)
						{
							tt = tt + dwellTime;
						}
						nextStopIt++;
						if(nextStopIt == busRouteStops.end()) { nextStop = nullptr; }
						else { nextStop = *nextStopIt; }
					}

					edgeTravelTime = edgeTravelTime + tt;
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
				break;
			}
			case sim_mob::TRAIN_EDGE:
			{
				double edgeTravelTime =  edge.getLinkTravelTimeSecs() + edge.getWalkTimeSecs() + edge.getWaitTimeSecs();
				edge.setLinkTravelTimeSecs(edgeTravelTime);
				pathTravelTime = pathTravelTime + edgeTravelTime;
				nextStartTime = DailyTime(nextStartTime.getValue() + std::floor(edgeTravelTime*1000));
				/*std::string startStop=edge.getStartStop();
				std::string endStop=edge.getEndStop();
				std::vector<std::string> stopVector=RailTransit::getInstance().fetchBoardAlightStopSeq(startStop,endStop);
				std::vector<std::string>::iterator it;
				bool inter=true;
				for(it=stopVector.begin() ; it < stopVector.end(); it++)
				{
                     if(inter==true)
                     {

                    	 inter=false;
                     }
                     else
                         inter=true;
				}*/
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
		pathSet.pathSet.insert(path);
	}
	pathSet.computeAndSetPathSize();
}

}
