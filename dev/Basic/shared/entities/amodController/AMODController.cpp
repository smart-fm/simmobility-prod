//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * AMODController.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: Max
 */

#include "AMODController.hpp"
#include <algorithm>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <ctime>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/roles/Role.hpp"
#include "entities/vehicle/VehicleBase.hpp"
#include "geospatial/network/Link.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "metrics/Frame.hpp"
#include "path/PathSetManager.hpp"
#include "workers/Worker.hpp"

using namespace std;

namespace sim_mob {
namespace AMOD
{
AMODController* AMODController::pInstance = nullptr;

sim_mob::AMOD::AMODController::~AMODController() {
	//close open files if necessary
	if (out_demandStat.is_open() )
	{
		out_demandStat.close();
	}

	if (out_vhsStat.is_open())
	{
		out_vhsStat.close();
	}

	if (out_tripStat.is_open())
	{
		out_tripStat.close();
	}
}

void sim_mob::AMOD::AMODController::init()
{
/*	stdir = &StreetDirectory::Instance();

	const sim_mob::RoadNetwork& roadNetwork = ConfigManager::GetInstance().FullConfig().getNetwork();
	const std::vector<Node*>& multiNodesPool = roadNetwork.getNodes();
	const std::set<sim_mob::UniNode*>& uniNodesPool = roadNetwork.getUniNodes();

	for(std::vector<sim_mob::Link *>::const_iterator it = roadNetwork.getLinks().begin(), it_end(roadNetwork.getLinks().end()); it != it_end ; it ++)
	{
		for(std::set<sim_mob::RoadSegment *>::iterator seg_it = (*it)->getUniqueSegments().begin(), it_end((*it)->getUniqueSegments().end()); seg_it != it_end; seg_it++)
		{
			if (!(*seg_it)->originalDB_ID.getLogItem().empty())
			{
				string aimsunId = (*seg_it)->originalDB_ID.getLogItem();
				string segId = getNumberFromAimsunId(aimsunId);
				segPool.insert(std::make_pair(segId,*seg_it));
			}
		}
	}

	for(std::vector<Node *>::const_iterator itMultiNodes = multiNodesPool.begin(); itMultiNodes != multiNodesPool.end(); ++itMultiNodes)
	{
		const sim_mob::Node* n = (*itMultiNodes);
		if (!n->originalDB_ID.getLogItem().empty())
		{
			std::string aimsunId = n->originalDB_ID.getLogItem();
			std::string id = getNumberFromAimsunId(aimsunId);
			nodePool.insert(std::make_pair(id,n));
		}
	}
	for(std::set<sim_mob::UniNode*>::const_iterator it=uniNodesPool.begin(); it!=uniNodesPool.end(); ++it)
	{
		const sim_mob::Node* n = (*it);
		if (!n->originalDB_ID.getLogItem().empty())
		{
			std::string aimsunId = n->originalDB_ID.getLogItem();
			std::string id = getNumberFromAimsunId(aimsunId);
			nodePool.insert(std::make_pair(id,n));
		}
	}
*/
}

void AMODController::registerController(int id, const MutexStrategy& mtxStrat)
{
	if(pInstance) {
		delete pInstance;
	}

	pInstance = new AMODController(id, mtxStrat);

}

AMODController* AMODController::instance()
{
	if (!pInstance) {
		pInstance = new AMODController();
	}

	return pInstance;
}

void AMODController::deleteInstance()
{
	safe_delete_item(pInstance);
}

bool AMODController::instanceExists()
{
	return pInstance;
}

AMODController::AMODController(int id,
		const MutexStrategy& mtxStrat)
: Agent(mtxStrat, id),frameTicks(0)
{
	init();
}

bool AMODController::frame_init(timeslice now)
{
	test=0;
	nFreeCars= 0; //initially, no free cars (before populate)
	return true;
}

void AMODController::precomputeAllPairsShortestPaths()
{
	std::map<std::string,const sim_mob::Node*>::iterator origItr;
	std::map<std::string,const sim_mob::Node*>::iterator destItr;

	Print() << "Pre-computing shortest paths" << std::endl;

	for (origItr = nodePool.begin(); origItr != nodePool.end(); origItr++) {
		Print() << "Node: " << origItr->first << std::endl;
		for (destItr = nodePool.begin(); destItr != nodePool.end(); destItr++) {
			std::string nodeKey;

			nodeKey = string(origItr->first) + "-" + string(destItr->first);
			std::vector < WayPoint > wp;
			if (origItr->second == destItr->second) {
				shortestPaths.insert(std::make_pair(nodeKey, wp));
				continue;
			}

			// compute shortest path
			std::vector<const sim_mob::RoadSegment*> blacklist; //empty vector -> nothing blacklisted

			/*std::vector < WayPoint > wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*(origItr->second)), stdir->DrivingVertex(*(destItr->second)),blacklist);
			for (int i=0; i<wp2.size(); i++) {
				if (wp2[i].type == WayPoint::ROAD_SEGMENT ) {
					wp.push_back(wp2[i]);
				}
			}*/

			shortestPaths.insert(std::make_pair(nodeKey, wp));
		}
	}
	return;
}

bool AMODController::hasShortestPath(std::string origNodeID, std::string destNodeID)
{
	bool doesShortestPathExist = false;
	std::string nodeKey = origNodeID + "-" + destNodeID;

	boost::unordered_map<std::string, vector<WayPoint> >::iterator spItr = shortestPaths.find(nodeKey);
	if (spItr == shortestPaths.end())
	{
		std::vector<WayPoint> wp;

		const Node* origNode = nodePool.at(origNodeID);
		const Node* destNode = nodePool.at(destNodeID);

		if (origNode == destNode)
		{
			shortestPaths.insert(std::make_pair(nodeKey, wp));
			doesShortestPathExist = true;
		}

		// compute shortest path
		std::vector<WayPoint> wp2 = stdir->SearchShortestDrivingPath(*origNode, *destNode);

		for (std::vector<WayPoint>::iterator itWayPts = wp2.begin(); itWayPts != wp2.end(); ++itWayPts)
		{
			if ((*itWayPts).type == WayPoint::ROAD_SEGMENT)
			{
				wp.push_back(*itWayPts);
			}
		}

		if (!wp.empty())
		{
			shortestPaths.insert(std::make_pair(nodeKey, wp));
			doesShortestPathExist = true;
		}
	}
	else
	{
		doesShortestPathExist = true;
	}

	return doesShortestPathExist;
}

vector <WayPoint> AMODController::getShortestPath(std::string origNodeID, std::string destNodeID)
{
	std::string nodeKey = origNodeID + "-" + destNodeID;
	boost::unordered_map<std::string, vector < WayPoint > >::iterator spItr = shortestPaths.find(nodeKey);
	if (spItr == shortestPaths.end()) {
		std::vector < WayPoint > wp;

		const Node* origNode = nodePool.at(origNodeID);
		const Node* destNode = nodePool.at(destNodeID);


		if (origNode == destNode) {
			shortestPaths.insert(std::make_pair(nodeKey, wp));
			return wp;
		}

		// compute shortest path
		std::vector<const sim_mob::RoadSegment*> blacklist;

		std::vector < WayPoint > wp2 = stdir->SearchShortestDrivingPath(*origNode, *destNode);
		/*std::vector<WayPoint> wp2 = stdir->SearchShortestDrivingTimePath(
					stdir->DrivingTimeVertex(*origNode,sim_mob::Default),
					stdir->DrivingTimeVertex(*destNode,sim_mob::Default),
					blacklist,
					sim_mob::Default);*/
		for (int i=0; i<wp2.size(); i++) {

			if (wp2[i].type == WayPoint::ROAD_SEGMENT ) {
				wp.push_back(wp2[i]);
			}
		}
		shortestPaths.insert(std::make_pair(nodeKey, wp));
		return wp;
	}
	return spItr->second;
}

vector <WayPoint> AMODController::getShortestPathWBlacklist(std::string origNodeID, std::string destNodeID, std::vector<const sim_mob::RoadSegment*> blacklist)
{
	std::vector < WayPoint > wp;
/*
	const Node* origNode = nodePool.at(origNodeID);
	const Node* destNode = nodePool.at(destNodeID);

	if (origNode == destNode) {
		return wp;
	}
	// compute shortest path
	// std::vector < WayPoint > wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*origNode), stdir->DrivingVertex(*destNode), blacklist);
	std::vector<WayPoint> wp2 = stdir->SearchShortestDrivingTimePath(
						stdir->DrivingTimeVertex(*origNode,sim_mob::Default),
						stdir->DrivingTimeVertex(*destNode,sim_mob::Default),
						blacklist,
						sim_mob::Default);
	for (int i=0; i<wp2.size(); i++) {
		if (wp2[i].type == WayPoint::ROAD_SEGMENT ) {
			wp.push_back(wp2[i]);
		}
	}
*/
	return wp;
}

void AMODController::calculateDistancesBetweenCarparks(std::vector<std::string> carParkIds, std::vector<std::vector < double > >& distancesBetweenCarparks)
{
	for (int i =0; i < carParkIds.size(); i++){
		string carParkOrigin = carParkIds[i];
		vector<double> vect_temp; //stores distances from i to all j
		for (int j =0; j < carParkIds.size(); j++){
			string carParkDest = carParkIds[j];
			if(i == j)
			{
				vect_temp.push_back(0.0);
			}else
			{
				vector<WayPoint> sp = getShortestPath(carParkOrigin, carParkDest);
				double tripDistance = 0;
				WayPoint iterWP;
				const RoadSegment *iterwp;
				for (int k = 0; k< sp.size(); k++)
				{
					iterWP = sp[k];
					tripDistance += sp[j].roadSegment->getLength();
				}
				vect_temp.push_back(tripDistance); //inCM
			}
		}
		distancesBetweenCarparks.push_back(vect_temp);
	}
}


Entity::UpdateStatus AMODController::frame_tick(timeslice now)
{
	int current_time = now.ms();
	currTime = current_time;
	emptyTripId = "e0"; // id for rebalancing vehicles
	if(test==0)
	{
		startRunTime = std::time(NULL);

		//load in configuration file
		ifstream amodConfigFile("amodConfig.cfg");
		std::string demandFileName;
		std::string carParkFileName;
		std::string outDemandFileName;
		std::string outVhsStatFilename;
		std::string outTripStatFilename;

		amodConfigFile >> demandFileName
		>> carParkFileName
		>> outDemandFileName
		>> outVhsStatFilename
		>> outTripStatFilename
		>> nCarsPerCarPark
		>> vehStatOutputModulus
		>> rebalancingModulus;

		myFile.open(demandFileName.c_str());
		carParkFile.open(carParkFileName.c_str());
		out_demandStat.open(outDemandFileName.c_str());
		out_vhsStat.open(outVhsStatFilename.c_str());
		out_tripStat.open(outTripStatFilename.c_str());

		populateCarParks(nCarsPerCarPark);

		//	calculateDistancesBetweenCarparks(carParkIds, distancesBetweenCarparks);

		lastReadLine = "";

		if (out_demandStat.is_open()) {
			out_demandStat << "tripID " <<
					"origin " << "destination " <<
					"timeRequested "
					<< "accepted/rejected " << "reasonRejected "
					<< "timePickedUp " << "TimeDropped " << "tripDistance\n" ;
		}
		else {
			Print() << "Unable to open out_demandStat.txt" << std::endl;
		}

		if (out_vhsStat.is_open()) {
			out_vhsStat << "vehId " << "time "
					<< "posX " << "posY "
					<< "Velocity " << "Acceleration "
					<< "onRoad " << "segmentId "
					<< "inCarpark " << "nodeId"
					<< std::endl;
		}
		else {
			Print() << "Unable to open out_vhsStat.txt" << std::endl;
		}

		if (out_tripStat.is_open()) {
			out_tripStat << "TripID" << " " << "Origin" << " "
					<< "Destination" << " " << "CarPark" << " "<< "AssignedAMODID" << " "
					<< "RequestTime" << " " << "DispatchTime" << " "
					<< "PickUpTime" << " " << "ArrivalTime" << " "
					<< "TripDistance"
					<< std::endl;
		} else {
			Print() << "Unable to open out_tripStat.txt" << std::endl;
		}

		test=1;
	}

	if(test==1)
	{
		//reading the file of time, destination and origin
		std::vector<string> origin;
		std::vector<string> destination;
		std::vector<string> tripID;

		readDemandFile(tripID, current_time, origin, destination);
		//mtx_.lock();
		if (currTime % vehStatOutputModulus == 0) {
			//saveVehStat();
		}
		//mtx_.unlock();
		assignVhsFast(tripID, origin, destination, current_time);

		if (currTime % rebalancingModulus == 0) {
			//rebalanceVhs();
		}

		//output the current running time
		Print() << "-----------------------------\n";
		Print() << "Current Simulated Time: " << ((double) currTime)/1000.0 << " s, "
				<< "[ " << ((double) currTime)/(60000.0) << " mins ]"
				<< std::endl;
		int elapsedTime = (int) (std::time(NULL) -  startRunTime);
		Print() << "Elapsed Running Time  : " << elapsedTime << " s, "
				<< "[ " << ((double) elapsedTime)/(60.0) << " mins ]"
				<< std::endl;
		Print() << "-----------------------------\n";
		test = 1;
	}
	// return continue, make sure agent not remove from main loop
	return Entity::UpdateStatus::Continue;
}

void AMODController::populateCarParks(int numberOfVhsAtNode = 1000)
{
	//carpark population
	//std::vector<std::string> carParkIds;

	while(!carParkFile.eof())
	{
		std::string line;
		std::getline (carParkFile,line);
		if (line == "") break;

		carParkIds.push_back(line);
	}

	int k = 0;

	for(int j = 0; j<carParkIds.size(); j++)
	{
		for(int i = 0; i<numberOfVhsAtNode; i++)
		{
			std::string vhId = "";
			string uId;          // string which will contain the result
			ostringstream convert;   // stream used for the conversion
			convert << k;      // insert the textual representation of 'Number' in the characters in the stream
			k++;
			uId = convert.str(); // set 'Result' to the contents of the stream
			vhId += uId;
			Print() << vhId << std::endl;

			std::string carParkId = carParkIds[j];
			Print() << "Inserting to the carpark: " << carParkId << std::endl;
			addNewVh2CarPark(vhId,carParkId);
		}
	}
}

void AMODController::readDemandFile(vector<string>& tripID, int current_time, vector<string>& origin, vector<string>& destination)
{
	string line;
	string tripID_;
	string time_;
	string origin_;
	string destination_;

	if (myFile.is_open())
	{
		while(!myFile.eof())
		{
			if(lastReadLine =="")
			{
				std::getline (myFile,line);
			}
			else{
				line = lastReadLine;
				lastReadLine = "";
			}
			if (line == "") break;

			std::stringstream ss;
			ss << line;
			ss >> tripID_ >> time_ >> origin_ >> destination_;
			int time = atoi(time_.c_str());

			if ((time*1000) <= current_time)
			{
				tripID.push_back(tripID_);
				origin.push_back(origin_);
				destination.push_back(destination_);
				lastReadLine = "";
			} else {
				lastReadLine = line;
				break;
			}

		}
	}
	else {
		Print() << "Unable to open file" << endl;
	}
}

void AMODController::addNewVh2CarPark(std::string& id,std::string& nodeId)
{
	// find node
	const Node* node = getNodeFrmPool(nodeId);

	if(node == NULL){ throw std::runtime_error("node not found"); }

	int startTime = currTime;

	// create person
	DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue() + startTime);
	sim_mob::Trip* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", node, "node", node, "node");
	sim_mob::SubTrip subTrip("", "Trip", 0, 1, start, DailyTime(), node, "node", node, "node", "Car");
	tc->addSubTrip(subTrip);
	std::vector<sim_mob::TripChainItem*>  tcs;
	tcs.push_back(tc);

	sim_mob::Person* person = new sim_mob::Person("AMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
	if (!person) {
		Print() << "Cannot create person!" << std::endl;
		return;
	}

	person->parentEntity = this;
	person->amodId = id;
	person->parkingNode = nodeId;
	person->initSpeed = 0;

	// add to virtual car park
	carParksMutex.lock();
	AMODVirtualCarParkItor it = virtualCarPark.find(nodeId);
	if(it!=virtualCarPark.end())
	{
		// access this car park before
		boost::unordered_map<std::string,Person*> cars = it->second;
		cars.insert(std::make_pair(id,person));

		it->second = cars;
	}
	else
	{
		boost::unordered_map<std::string,Person*> cars = boost::unordered_map<std::string,Person*>();
		cars.insert(std::make_pair(id,person));
		virtualCarPark.insert(std::make_pair(nodeId,cars));
	}

	//insert this car into the global map of all cars in car parks
	vhInCarPark.insert( std::make_pair(id, person) );
	carParksMutex.unlock();

	allAMODCarsMutex.lock();
	allAMODCars[id]= person; //add this car to the global map of all AMOD cars (for bookkeeping)
	nFreeCars++;
	allAMODCarsMutex.unlock();

	person->currStatus = Person::IN_CAR_PARK;
}


bool AMODController::getBestFreeVehicle(std::string originId, sim_mob::Person **vh, std::string &carParkIdDeparture, std::vector < sim_mob::WayPoint > &leastCostPath, double &bestTravelCost) {

	// initialize our vars
	AMODVirtualCarParkItor iter;
	AMODVirtualCarParkItor bestCarParkIter;
	*vh = NULL;
	bool freeCarFound = false;
	bestTravelCost = -1;
	bestCarParkIter = virtualCarPark.begin();

	//find the closest car park
	carParksMutex.lock();
	std::vector < std::vector < sim_mob::WayPoint > > carParksToOriginWaypoints;
	for (iter=virtualCarPark.begin(); iter != virtualCarPark.end(); iter++) {
		boost::unordered_map<std::string,Person*> cars = iter->second;
		if (!cars.empty()) {
			//this car park has cars, find distance to node
			double travelCost;

			vector < sim_mob::WayPoint > wps;

			if (iter->first == originId) {
				travelCost = 0; //we are at the same place
				bestTravelCost = travelCost;
				bestCarParkIter = iter;
				leastCostPath = wps;
				freeCarFound = true;
				continue;
			}

			travelCost = getTravelTimePath(iter->first , originId, wps);

			//if the travel cost is less than the best (or no car found yet), assign new best
			if (wps.size() > 0) { //make sure it is a valid road segment path
				if (freeCarFound) {
					if (travelCost < bestTravelCost) {
						bestTravelCost = travelCost;
						bestCarParkIter = iter;
						leastCostPath = wps;
					}
				} else {
					bestTravelCost = travelCost;
					bestCarParkIter = iter;
					leastCostPath = wps;
					freeCarFound = true;
				}
			}
		} else {
			//this car park is empty
			continue;
		}
	}

	// grab a vehicle from the car park, set the carParkId, way points and return
	if (freeCarFound) {
		carParkIdDeparture = bestCarParkIter->first;
		boost::unordered_map<std::string,Person*> cars = bestCarParkIter->second;
		boost::unordered_map<std::string,Person*>::iterator firstCarIt = cars.begin();
		freeCarFound = false;
		for (firstCarIt = cars.begin(); firstCarIt != cars.end(); firstCarIt++) {
			if (! firstCarIt->second->isToBeRemoved()) {
				*vh = firstCarIt->second;
				freeCarFound = true;
				break;
			}
		}
	}
	carParksMutex.unlock();
	return freeCarFound;
}

double AMODController::getTravelTimePath(std::string startNodeId, std::string endNodeId, vector < sim_mob::WayPoint > &leastCostPath) {

	const sim_mob::Node* startNode = nodePool.at(startNodeId);
	const sim_mob::Node* endNode = nodePool.at(endNodeId);

	std::vector<WayPoint> wp = getShortestPath(startNodeId, endNodeId);

	for(int i=0;i<wp.size();++i)
	{
		if(wp[i].type == WayPoint::ROAD_SEGMENT )
		{
			leastCostPath.push_back(wp[i]);
		}
	}

	return calculateTravelTime( leastCostPath ); //return the travel cost of the path
}


//finds the nearest free vehicle and returns the carParkId where the vehicle was found and a pointer to the vehicle
bool AMODController::findNearestFreeVehicle(std::string originId, std::map<std::string, sim_mob::Node* > &nodePool, std::string &carParkId, Person **vh) {
	// get the origin node
	Node *originNode = nodePool[originId];

	// get the positions of the nodes
	double oX = originNode->getLocation()->getX();
	double oY = originNode->getLocation()->getY();

	std::vector< distPair > nodeDistances;

	// loop through nodePool
	typedef std::map<std::string, sim_mob::Node*>::iterator it_type;
	for(it_type iterator = nodePool.begin(); iterator != nodePool.end(); iterator++) {
		std::string nodeId = iterator->first;
		Node *tempNode = iterator->second;

		double tX = tempNode->getLocation()->getX();
		double tY = tempNode->getLocation()->getY();

		double dist = pow( tX - oX,2) + pow( tY - oY, 2);
		nodeDistances.push_back( distPair(dist, nodeId) );
	}
	// sort the vector
	std::sort(nodeDistances.begin(), nodeDistances.end(), boost::bind(&AMODController::distPairComparator, this, _1, _2));

	// loop through the sorted distances and check for a free car in each node
	*vh = NULL;
	typedef std::vector< distPair >::iterator dt_type;
	for (dt_type iterator = nodeDistances.begin(); iterator != nodeDistances.end(); iterator++) {
		carParkId = iterator->second;

		if(!getVhFromCarPark( carParkId, &(*vh) )) {
			continue;
		} else {
			return true;
		}
	}
	// no car found
	return false;
}

bool AMODController::getVhFromCarPark(std::string& carParkId,Person** vh)
{
	carParksMutex.lock();
	AMODVirtualCarParkItor it = virtualCarPark.find(carParkId);
	if(it==virtualCarPark.end()){
		carParksMutex.unlock();
		return false;
	}

	boost::unordered_map<std::string,Person*> cars = it->second;
	if(!cars.empty())
	{
		boost::unordered_map<std::string,Person*>::iterator firstCarIt = cars.begin();
		*vh = firstCarIt->second;
		cars.erase(firstCarIt);
		it->second = cars;
		Print() << "Cars size: " << cars.size() << std::endl;
		vhInCarPark.erase( (*vh)->amodId );
		(*vh)->currStatus = Person::ON_THE_ROAD;
		nFreeCars--;
		carParksMutex.unlock();
		return true;
	}
	carParksMutex.unlock();
	return false;
}

bool AMODController::removeVhFromCarPark(std::string& carParkId,Person** vh)
{
	if ((*vh)->isToBeRemoved()) {
		return false;
	}
	carParksMutex.lock();
	AMODVirtualCarParkItor it = virtualCarPark.find(carParkId);
	if(it==virtualCarPark.end()){
		carParksMutex.unlock();
		return false;
	}

	boost::unordered_map<std::string,Person*> cars = it->second;
	if(!it->second.empty())
	{
		if (cars.find((*vh)->amodId) == cars.end()) {
			carParksMutex.unlock();
			return false;
		}
		cars.erase((*vh)->amodId);
		it->second = cars;
		vhInCarPark.erase( (*vh)->amodId );
		(*vh)->currStatus = Person::ON_THE_ROAD;
		nFreeCars--;
		carParksMutex.unlock();
		return true;
	}
	carParksMutex.unlock();
	return false;
}


void AMODController::mergeWayPoints(const std::vector<sim_mob::WayPoint>& carparkToOrigin, const std::vector<sim_mob::WayPoint> &originToDestination, std::vector<sim_mob::WayPoint>& mergedWP)
{
	for (int i=0; i<carparkToOrigin.size(); i++) {
		if (carparkToOrigin[i].type == WayPoint::ROAD_SEGMENT ) {
			mergedWP.push_back(carparkToOrigin[i]);
		}
	}

	for (int i=0; i<originToDestination.size(); i++) {
		if (originToDestination[i].type == WayPoint::ROAD_SEGMENT )  {
			mergedWP.push_back(originToDestination[i]);
		}
	}
}

bool AMODController::dispatchVh(Person* vh)
{
	this->currWorkerProvider->scheduleForBred(vh);
}


void AMODController::handleVHDestruction(Person *vh)
{
	//if a vehicle gets destroyed by Simmobility, we have to make sure we're not still assuming it exists.
	if (vh->currStatus == Person::REPLACED) {
		//we have already replaced this guy. don't bother doing anything.
		return;
	}

	std::string amodId = vh->amodId;
	Print() << vh->amodId << " is being destroyed." << std::endl;

	boost::unordered_map<std::string,Person*>::iterator itr;
	allAMODCarsMutex.lock();
	itr = allAMODCars.find(amodId);
	if (itr == allAMODCars.end()) {
		Print() << "Already removed" << std::endl;
		return;
	} else {
		if (itr->second == vh) {
			//the vehicle is still in the AMODController, we have to remove it and reinsert it into a location
			AmodTrip atrip = vhTripMap[vh];
			atrip.arrivalTime = currTime;
			atrip.tripError = true;
			saveTripStat(atrip);

			allAMODCars.erase(itr);
			allAMODCarsMutex.unlock();

			vhOnTheRoad.erase(vh->amodId);

			vhTripMapMutex.lock();
			vhTripMap.erase(vh);
			vhTripMapMutex.unlock();

			vh->currStatus = Person::REPLACED;
		} else {
			//all is good, do nothing for now
		}
	}
}

void AMODController::processArrival(Person *vh)
{
	if (vhTripMap.find(vh) == vhTripMap.end()) {
		//this is not an AMOD vehicle
		return;
	}

	AmodTrip atrip = vhTripMap[vh];

	Print() << vh->amodId << " arriving at " << atrip.destination << std::endl;

	atrip.arrivalTime = currTime;
	saveTripStat(atrip);
	vhTripMap.erase(vh);

	//mtx_.lock();

	std::string vhID = vh->amodId;

	allAMODCarsMutex.lock();
	allAMODCars.erase(vh->amodId);
	vhOnTheRoad.erase(vh->amodId);
	allAMODCarsMutex.unlock();

	addNewVh2CarPark(vhID,atrip.destination);
	//mtx_.unlock();
}

void AMODController::handleVHPickup(Person *vh) {
	TripMapIterator itr = vhTripMap.find(vh);
	if (itr == vhTripMap.end()) {
		//this is not an AMOD vehicle
		return;
	}
	//if not already picked up, then set the picked up time
	if (!(itr->second.pickedUp)) {
		itr->second.pickedUp = true;
		itr->second.pickUpTime = currTime;
	}
}

void AMODController::handleVHArrive(Person* vh)
{
	processArrival(vh);
}

void AMODController::rerouteWithPath(Person* vh,std::vector<sim_mob::WayPoint>& path)
{
	AMODRerouteEventArgs arg(NULL,NULL,path);
	eventPub.publish(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH, vh, arg);
}

void AMODController::rerouteWithOriDest(Person* vh,Node* snode,Node* enode)
{
	AMODRerouteEventArgs arg(snode,enode,std::vector<WayPoint>());
	eventPub.publish(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_ORI_DEST, vh, arg);
}

bool AMODController::setPath2Vh(Person* vh,std::vector<WayPoint>& path)
{
	vh->setPath(path);
}

//TODO: use uniform method for storing and retrieving travel times - Vahid
#if 0
void AMODController::setRdSegTravelTimes(Person* ag, double rdSegExitTime) {

	std::map<double, Person::RdSegTravelStat>::const_iterator it =
			ag->getRdSegTravelStatsMap().find(rdSegExitTime);

	ofstream out_TT;
	out_TT.open("out_TT.txt", fstream::out | fstream::app);

	if (it != ag->getRdSegTravelStatsMap().end()){
		double travelTime = (it->first) - (it->second).entryTime;
		std::map<const RoadSegment*, Conflux::RdSegTravelTimes>::iterator itTT = RdSegTravelTimesMap.find((it->second).rs);
		if (itTT != RdSegTravelTimesMap.end())
		{
			itTT->second.agCnt = itTT->second.agCnt + 1;
			itTT->second.travelTimeSum = itTT->second.travelTimeSum + travelTime;
		}
		else{
			Conflux::RdSegTravelTimes tTimes(travelTime, 1);
			RdSegTravelTimesMap.insert(std::make_pair(ag->getCurrSegment(), tTimes));
		}

		WayPoint w = ag->amodPath.back();
		const RoadSegment *rs = w.roadSegment_;
		std::string segmentID = rs->originalDB_ID.getLogItem();

		Print() << "Segment ID: "<< segmentID << " ,Segment travel time: " << rdSegExitTime << std::endl;

		if (out_TT.is_open()) {
			out_TT << "Segment ID: " << segmentID << "Segment travel time: " << rdSegExitTime << std::endl;
		}
		else{
			Print() << "Unable to open file\n";
		}
	}
	if (out_TT.is_open()) out_TT.close();
}

void AMODController::updateTravelTimeGraph()
{
	const unsigned int msPerFrame = ConfigManager::GetInstance().FullConfig().baseGranMS();
	timeslice currTime = timeslice(currTick.frame(), currTick.frame()*msPerFrame);
	insertTravelTime2TmpTable(currTime, RdSegTravelTimesMap);
}

bool AMODController::insertTravelTime2TmpTable(timeslice frameNumber, std::map<const RoadSegment*, sim_mob::Conflux::RdSegTravelTimes>& rdSegTravelTimesMap)
{
	bool res=false;
	if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
		//sim_mob::Link_travel_time& data
		std::map<const RoadSegment*, sim_mob::Conflux::RdSegTravelTimes>::const_iterator it = rdSegTravelTimesMap.begin();
		for (; it != rdSegTravelTimesMap.end(); it++){
			LinkTravelTime tt;
			DailyTime simStart = ConfigManager::GetInstance().FullConfig().simStartTime();
			std::string aimsun_id = (*it).first->originalDB_ID.getLogItem();
			std::string seg_id = getNumberFromAimsunId(aimsun_id);
			try {
				tt.linkId = boost::lexical_cast<int>(seg_id);
			} catch( boost::bad_lexical_cast const& ) {
				Print() << "Error: seg_id string was not valid" << std::endl;
				tt.linkId = -1;
			}

			tt.startTime = (simStart + sim_mob::DailyTime(frameNumber.ms())).getStrRepr();
			double frameLength = ConfigManager::GetInstance().FullConfig().baseGranMS();
			tt.endTime = (simStart + sim_mob::DailyTime(frameNumber.ms() + frameLength)).getStrRepr();
			tt.travelTime = (*it).second.travelTimeSum/(*it).second.agCnt;

			PathSetManager::getInstance()->insertTravelTime2TmpTable(tt);
		}
	}
	return res;
}
#endif


void AMODController::testTravelTimePath()
{/*
	std::string destNodeId="61688";
	std::string carParkId = "75780";
	const Node *startNode = nodePool.at(carParkId);
	const Node *endNode = nodePool.at(destNodeId);

	std::vector<const sim_mob::RoadSegment*> blacklist = std::vector<const sim_mob::RoadSegment*>();

	std::vector<WayPoint> wp = stdir->SearchShortestDrivingTimePath(
			stdir->DrivingTimeVertex(*startNode,sim_mob::Default),
			stdir->DrivingTimeVertex(*endNode,sim_mob::Default),
			blacklist,
			sim_mob::Default);
	for(int i=0;i<wp.size();++i)
	{
		if(wp[i].type == WayPoint::ROAD_SEGMENT)
		{
			const RoadSegment* rs = wp[i].roadSegment;
			//Print() <<"from node: "<<rs->getStart()->originalDB_ID.getLogItem()<<" to node: "<<rs->getEnd()->originalDB_ID.getLogItem()<<std::endl;
		}
	}*/
}

void AMODController::handleAMODEvent(sim_mob::event::EventId id,
		sim_mob::event::Context ctxId,
		sim_mob::event::EventPublisher* sender,
		const AMOD::AMODEventArgs& args)
{
	if(id == event::EVT_AMOD_ARRIVING_TO_DEST)
	{
		//TODO
	}
}


double AMODController::calculateTravelTime(std::vector < sim_mob::WayPoint > &wPs ) {
	// loop through all way points, get time for each segment and add to total
	double travelTime = 0;
	std::vector < sim_mob::WayPoint >::iterator wPIter;
	for (wPIter = wPs.begin(); wPIter != wPs.end(); wPIter++) {
		// get segment and get travel time
		if ((*wPIter).type == WayPoint::ROAD_SEGMENT) {
			const RoadSegment *rs = (*wPIter).roadSegment;
			travelTime += rs->getLength()/(rs->getMaxSpeed()*27.778);
		}
	}
	return travelTime;
}

void AMODController::saveTripStat(AmodTrip &a) {
	if (out_tripStat) {
		if (!(out_tripStat.is_open()))
			return;

		//save data to file
		out_tripStat << a.tripID << " " << a.origin << " "
				<< a.destination << " " << a.carParkIdDeparture << " "
				<< a.assignedAmodId << " "
				<< a.requestTime << " " << a.dispatchTime << " "
				<< a.pickUpTime << " " << a.arrivalTime << " "
				<< a.tripDistanceInM << " " << a.tripError
				<< std::endl;
	}

	return;
}

void AMODController::unregisteredChild(Entity* child)
{
	Person *person = dynamic_cast<Person*>(child);
	if(person){
		TripMapIterator itr = vhTripMap.find(person);
		if(itr!=vhTripMap.end()){
			handleVHDestruction(person);
		}
	}
}

const Node* AMODController::getNodeFrmPool(const std::string& nodeId) {
	std::map<std::string,const sim_mob::Node*>::iterator itNode = nodePool.find(nodeId);
	const Node* result = nullptr;
	if (itNode != nodePool.end()) {
		result = itNode->second;
	}
	return result;
}

void AMODController::blacklistForbiddenSegments(const RoadSegment* lastWPrs, const Node* startNode, std::vector<const sim_mob::RoadSegment*> &blacklist)
{
	/*
	//check if multi node
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*>(startNode);
	if(currEndNode) {
		// it is multi node
		//find all segments you can go from the node
		const std::set<sim_mob::RoadSegment*> allSeg = currEndNode->getRoadSegments();
		std::set<sim_mob::RoadSegment*>::const_iterator it;
		for(it = allSeg.begin(); it!= allSeg.end(); ++it){
			RoadSegment* rs = *it;
			if(rs != lastWPrs){
				blacklist.push_back(rs);
			}
		}

	}

	// check if uniNode
	const UniNode* currEndNodeUni = dynamic_cast<const UniNode*>(startNode);
	if(currEndNodeUni){
		//find all segments you can go from the node
		const std::vector<const sim_mob::RoadSegment*>& allSeg = currEndNodeUni->getRoadSegments();

		for(int i=0;i<allSeg.size();++i){
			const RoadSegment* rs = allSeg[i];
			if(rs != lastWPrs){
				blacklist.push_back(rs);
			}

//			std::cout << "Blacklist uniNode: " << std::endl;
//			for (int i=0; i<blacklist.size(); i++) {
//				std::cout << " -> " << blacklist[i]->originalDB_ID.getLogItem();
//			}
//			std::cout << std::endl;
		}
	}
	*/
}

// trip from node to carpark
void AMODController::findNearestCarPark(std::string& destinationNode, const RoadSegment* lastWPrs, std::vector<std::string>& AllCarParks, std::string &carParkIdArrival, vector<const sim_mob::RoadSegment*> &blacklist)
{
/*
	// calculate sp to all carParks and choose the nearest carpark
	string carPark_temp;
	vector <WayPoint> wp_temp;
	double shortestDistance=9999999999;

	for (int ii = 0; ii < AllCarParks.size(); ii++){
		carPark_temp = AllCarParks[ii];
		if(destinationNode==carPark_temp){
			carParkIdArrival = AllCarParks[ii];
			shortestDistance = 0.0;
			break;

		}else{
			// find shortest path to the destinationNode and calculate the distance
			//get blacklist
			//get the second last node as a new start node for the second part of the trip
			const Node* startNode = lastWPrs->getStart();
			//blacklist all segments coming from a given node except of one segment which is called StopPointRS, other segments are added to the blacklist
			blacklistForbiddenSegments(lastWPrs, startNode, blacklist);
			//calculate sp with blacklist
			std::string s1 = startNode->originalDB_ID.getLogItem();
			string startNodeId = getNumberFromAimsunId(s1);

			wp_temp = getShortestPathWBlacklist(startNodeId, carPark_temp, blacklist);

			// distance of the path
			WayPoint iterWP;
			double tripDistance = 0;
			for (int j = 0; j< wp_temp.size(); j++){
				iterWP = wp_temp[j];
				const RoadSegment *rs = iterWP.roadSegment_;
				tripDistance += wp_temp[j].roadSegment_->length;
			}
			// if the current distance is shorter than the previous, then carParkIdArrival = AllCarParks[ii];
			if (tripDistance < shortestDistance){
				shortestDistance = tripDistance;
				carParkIdArrival = AllCarParks[ii];
			}
		}
	}
*/
}

// trip from carpark to node
void AMODController::findNearestCarParkToNode(std::string& originNode, const RoadSegment* firstWPrs, std::vector<std::string>& AllCarParks, std::string &carParkIdArrival,  vector<const sim_mob::RoadSegment*> &blacklist)
{
/*	
	// calculate sp to all carParks and choose the nearest carpark
	string carPark_temp;
	vector <WayPoint> wp_temp;
	double shortestDistance=9999999999;

	for (int ii = 0; ii < AllCarParks.size(); ii++){
		carPark_temp = AllCarParks[ii];
		if(originNode==carPark_temp){
			carParkIdArrival = AllCarParks[ii];
			shortestDistance = 0.0;
			break;

		}else{

			const Node* endNode = firstWPrs->getEnd();
			vector<const sim_mob::RoadSegment*> blacklist;
			blacklistForbiddenSegments(firstWPrs, endNode, blacklist);

			//calculate sp with blacklist
			std::string s1 = endNode->originalDB_ID.getLogItem();
			string endNodeId = getNumberFromAimsunId(s1);

			wp_temp = getShortestPathWBlacklist(carPark_temp, endNodeId, blacklist);

			// distance of the path
			WayPoint iterWP;
			double tripDistance = 0;
			//std::cout << "Length of segments: ";
			for (int j = 0; j< wp_temp.size(); j++){
				iterWP = wp_temp[j];
				const RoadSegment *rs = iterWP.roadSegment;
				tripDistance += wp_temp[j].roadSegment_->length;
			}
			// if the current distance is shorter than the previous, then carParkIdArrival = AllCarParks[ii];
			if (tripDistance < shortestDistance){
				shortestDistance = tripDistance;
				carParkIdArrival = AllCarParks[ii];
			}
		}
	}
*/
}

void AMODController::calculateThePath(std::vector<WayPoint>& wp1, std::string& toNode, vector<const sim_mob::RoadSegment*> blacklist, std::string& pickUpSegmentStr, const RoadSegment* &StopPointRS, std::vector < sim_mob::WayPoint > &routeWP)
{
/*
	//get the last WayPoint from the wp1
	WayPoint lastWP = wp1[wp1.size()-1];
	const RoadSegment *lastWPrs = lastWP.roadSegment_;
	pickUpSegmentStr = lastWPrs->originalDB_ID.getLogItem();
	StopPointRS = lastWPrs; //this is the segment where picking up f the passenger will occur
	//erase the last WayPoint from the wp1
	wp1.pop_back();
	//get the second last node as a new start node for the second part of the trip
	const Node* startNode = lastWPrs->getStart();
	//blacklist all segments coming from a given node except of one segment which is called StopPointRS, other segments are added to the blacklist
	blacklistForbiddenSegments(lastWPrs, startNode, blacklist);
	//calculate sp with blacklist
	std::string s1 = startNode->originalDB_ID.getLogItem();
	string startNodeId = getNumberFromAimsunId(s1);

	std::vector<WayPoint> wpToOrigin = getShortestPathWBlacklist(startNodeId, toNode, blacklist);

	//if(wpToOrigin.size() == 0){
	//	continue;
	//}
	//merge wayPoints temp
	mergeWayPoints(wp1, wpToOrigin, routeWP);
*/
}

void AMODController::assignVhsFast(std::vector<std::string>& tripID, std::vector<std::string>& origin, std::vector<std::string>& destination, int currTime)
{
	if (out_demandStat.is_open()) {
		std::vector < Person * > justDispatched;

		//add origin and destination to complete set
		for (int i = 0; i < origin.size(); i++){
			AmodTrip atrip;
			atrip.tripID = tripID[i];
			atrip.origin = origin[i];
			atrip.destination = destination[i];
			atrip.requestTime = currTime;

			//check if there is a route from origin to destination
			std::vector < WayPoint > wp1 = getShortestPath(origin[i],
					destination[i]);
			if (wp1.size() == 0) {
				if (origin[i] == destination[i]) {
					out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "0\n"; //origin same as destination
				} else {
					out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "1\n"; //no path to destination
				}
				continue; //not possible to service this trip. no possible route.
			}

			bool validDemand = false;
			string carParkIdDepartureTest = "";
			string carParkIdArrivalTest = "";

			//check if there is a route from carpark to origin
			// I calculate the path from carpark to the second node from wp1
			// find first WayPoint from (origin to dest)
			WayPoint firstWP = wp1[0];
			const RoadSegment *firstWPrs = firstWP.roadSegment;
			vector<const sim_mob::RoadSegment*> blacklist;
			//find nearest carpark to the second node from the WP1
			findNearestCarParkToNode(origin[i], firstWPrs, carParkIds, carParkIdDepartureTest, blacklist);
			if (carParkIdDepartureTest != "") {
				validDemand = true;
				// break;
			} else {

				out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "2" << std::endl;
				continue;
			}

			//check if there is a route from destination to carpark
			// path from the second last node in WP1 to the nearest carpark
			// find the last wayPoint from the WP1 (origin to dest)
			WayPoint lastWP = wp1[wp1.size() - 1];
			const RoadSegment *lastWPrs = lastWP.roadSegment;
			//find nearest carpark from the second last node from the WP1
			findNearestCarPark(destination[i], lastWPrs, carParkIds, carParkIdArrivalTest, blacklist);
			if (carParkIdArrivalTest != "") {
				validDemand = true;
				//break;
			} else {
				out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "3" << std::endl;
				continue;
			}

			//
			//			AMODVirtualCarParkItor iter;
			//			//check if there is route from a car park to the origin
			//			for (iter=virtualCarPark.begin(); iter != virtualCarPark.end(); iter++) {
			//				wp0 = getShortestPath(iter->first, origin[i]);
			//				if (wp0.size() > 0) {
			//					validDemand = true;
			//					//				break;
			//				}
			//				if (!validDemand)
			//				{ // no path from carpark to origin
			//					out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "2" << std::endl;
			//					continue;
			//				}
			//				std::vector<WayPoint> wpToDest;
			//
			//				if(validDemand){ // check from (orgin-1) to dest
			//					//get the last WayPoint from the wp1
			//					WayPoint lastWP = wp0[wp0.size()-1];
			//					const RoadSegment *lastWPrs = lastWP.roadSegment_;
			//					wp0.pop_back();
			//					//get the second last node as a new start node for the second part of the trip
			//					const Node* startNode = lastWPrs->getStart();
			//					//blacklist all segments coming from a given node except of one segment which is called StopPointRS, other segments are added to the blacklist
			//					vector<const sim_mob::RoadSegment*> blacklist;
			//					blacklistForbiddenSegments(lastWPrs, startNode, blacklist);
			//					//calculate sp with blacklist
			//					std::string s1 = startNode->originalDB_ID.getLogItem();
			//					string startNodeId = getNumberFromAimsunId(s1);
			//					wpToDest = getShortestPathWBlacklist(startNodeId, destination[i], blacklist);
			//				}
			//
			//				validDemand = false;
			//				if (wpToDest.size() > 0) {
			//					validDemand = true;
			//					//			break;
			//				}
			//
			//				if (!validDemand)
			//				{// no path from (orginin-1) to dest
			//					out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "4" << std::endl;
			//					continue;
			//				}
			//
			//				string carParkIdArrivalTest = "";
			//				if(validDemand){ // check from (dest-1) to carpark
			//					//get the last WayPoint from the wp1
			//					WayPoint lastWP = wpToDest[wpToDest.size()-1];
			//					const RoadSegment *lastWPrs = lastWP.roadSegment_;
			//					wpToDest.pop_back();
			//					//get the second last node as a new start node for the second part of the trip
			//					const Node* startNode = lastWPrs->getStart();
			//					//blacklist all segments coming from a given node except of one segment which is called StopPointRS, other segments are added to the blacklist
			//					vector<const sim_mob::RoadSegment*> blacklist;
			//					blacklistForbiddenSegments(lastWPrs, startNode, blacklist);
			//					//calculate sp with blacklist
			//					std::string s1 = startNode->originalDB_ID.getLogItem();
			//					string startNodeId = getNumberFromAimsunId(s1);
			//
			//					// second iter for carpark after servicing the trip
			//					AMODVirtualCarParkItor iter2;
			//					//check if there is route from a car park to the origin
			//					findNearestCarPark(startNodeId, carParkIds, carParkIdArrivalTest);
			//
			//				}
			//				validDemand = false;
			//				if (carParkIdArrivalTest != "") {
			//					validDemand = true;
			//					break;
			//				}
			//				if (!validDemand)
			//				{// no path from (dest-1) to carpark
			//					out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "5" << std::endl;
			//					continue;
			//				}
			//			}

			//is a valid trip
			out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 1 " << "0" << std::endl;

			serviceBuffer.push_back(atrip);
		}

		std::cout << "AMOD Service Buffer Size: " << serviceBuffer.size() << ", Free Cars: " << nFreeCars << std::endl;

		// work through list using available free cars
		ServiceIterator itr =serviceBuffer.begin();
		int startTime = currTime;
		const RoadSegment *StopPointRS = NULL; //vh will stop at this segment to pick up passenger
		const RoadSegment *dropOffPointRS = NULL;
		string pickUpSegmentStr;
		string dropOffSegmentStr;
		while (true) {
			if (nFreeCars <= 0) break; //check if the number of free cars is non-zero
			if (serviceBuffer.empty()) break;
			if (itr == serviceBuffer.end()) break;

			std::string originNodeId = itr->origin;
			std::string destNodeId = itr->destination;
			int reqTime = itr->requestTime;
			std::string tripId = itr->tripID;

			const Node *originNode = getNodeFrmPool(originNodeId);
			const Node *destNode = getNodeFrmPool(destNodeId);

			//----------------------------------------------------------------------------------------
			// Search for the closest free AMOD vehicle

			std::vector < sim_mob::WayPoint > leastCostPath;
			double bestFreeVehTravelCost;
			Person* vhAssigned = nullptr;
			std::string carParkIdDeparture = "";
			std::string carParkIdArrival = "";
			bool freeVehFound = getBestFreeVehicle(originNodeId, &vhAssigned, carParkIdDeparture, leastCostPath, bestFreeVehTravelCost);
			if (!freeVehFound)
			{
				if (carParkIdDeparture == originNodeId) {
					itr++;
					continue;
				} else {
					bool carParkFound = false;
					AMODVirtualCarParkItor iter;
					for (iter=virtualCarPark.begin(); iter != virtualCarPark.end(); iter++) {
						if (hasShortestPath(iter->first,originNodeId)) {
							carParkFound = true;
							break;
						}
					}
					if (carParkFound) {
						itr++;
					} else {
						//this will never be serviceable
						out_demandStat << tripId << " " << reqTime << " " << originNodeId << " " << destNodeId << " " << reqTime << " 0 " << "2" << std::endl;
						itr = serviceBuffer.erase(itr);
					}
				}
				continue;
			}
			//----------------------------------------------------------------------------------------
			// Find nearest carPark to the destination node

			std::vector<WayPoint> wpC2 = getShortestPath(originNodeId, destNodeId);

			vector<const sim_mob::RoadSegment*> blacklist;
			WayPoint lastWP = wpC2[wpC2.size()-1];
			const RoadSegment *lastWPrs = lastWP.roadSegment;

			findNearestCarPark(destNodeId, lastWPrs, carParkIds, carParkIdArrival, blacklist);

			//----------------------------------------------------------------------------------------
			// Compute route for found AMOD vehicle

			const Node *carParkNodeDeparture = getNodeFrmPool(carParkIdDeparture);
			const Node *carParkNodeArrival = getNodeFrmPool(carParkIdArrival);
			if(carParkNodeDeparture==nullptr){
				Print()<<"some error should be happen in AmodController(carParkNode)"<< std::endl;
				return;
			}
			if(carParkNodeArrival==nullptr){
				Print()<<"some error should be happen in AmodController(carParkNode)"<< std::endl;
				return;
			}

			//----------------------------------------------------------------------------------------
			// get route for vehicle
			vector<WayPoint> mergedWP;
			//route from origin to destination
			std::vector<WayPoint> wp2 = getShortestPath(originNodeId, destNodeId); //shortest path sed in case when the carpark is at the same node as the origin
			//
			//			string carParkIdDeparture = "";
			//			string carParkIdArrival = "";
			//
			//			// I calculate the path from carpark to the second node from wp1
			//			// find first WayPoint from (origin to dest)
			//			WayPoint firstWP = wp2[0];
			//			const RoadSegment *firstWPrs = firstWP.roadSegment_;
			//			vector<const sim_mob::RoadSegment*> blacklist;
			//			//find nearest carpark to the second node from the WP1
			//			findNearestCarParkToNode(originNodeId, firstWPrs, carParkIds, carParkIdDeparture, blacklist);
			//			vector<WayPoint> wpToOrigin = getShortestPathWBlacklist(carParkIdDeparture, originNodeId, blacklist);
			//
			//			//check if there is a route from destination to carpark
			//
			//			// path from the second last node in WP1 to the nearest carpark
			//			// find the last wayPoint from the WP1 (origin to dest)
			//			WayPoint lastWP = wp2[wp2.size()-1];
			//			const RoadSegment *lastWPrs = lastWP.roadSegment_;
			//			//find nearest carpark from the second last node from the WP1
			//			findNearestCarPark(destNodeId, lastWPrs, carParkIds, carParkIdArrival, blacklist);
			//			vector<WayPoint> wpToCarpark = getShortestPathWBlacklist(destNodeId, carParkIdArrival, blacklist);
			//
			//			// merge all 3 sets of waypoints
			//			vector<WayPoint> mergedWP_temp;
			//			mergeWayPoints(wpToOrigin, wp2, mergedWP_temp);
			//			mergeWayPoints(mergedWP_temp, wpToCarpark, mergedWP);
			//
			//			// assign vh to the path, if no vhs at this carpark, then reject the trip
			//
			//
			//----------------------------------------------------------------------------------------







			//----------------------------------------------------------------------------------------
			if ((carParkNodeDeparture != originNode) && (carParkNodeArrival == destNode)) {
				//find route from carPark to origin and from origin to destination
				std::vector<WayPoint> wp1 = getShortestPath(carParkIdDeparture, originNodeId);
				vector<const sim_mob::RoadSegment*> blacklist;
				calculateThePath(wp1, destNodeId, blacklist, pickUpSegmentStr,
						StopPointRS, mergedWP);
//				std::cout << "carPark->origin->destination: WPs after merging:" << std::endl;
//				for (int i=0; i<mergedWP.size(); i++) {
//					std::cout << " -> " << mergedWP[i].roadSegment_->originalDB_ID.getLogItem();
//				}
//				std::cout << std::endl;
			} else if ((carParkNodeDeparture == originNode)
					&& (carParkNodeArrival != destNode)) { //find route from origin to destination and from dest to carPark and merge
				std::vector < WayPoint > wp2n = getShortestPath(originNodeId,
						destNodeId);
				vector<const sim_mob::RoadSegment*> blacklist;
				calculateThePath(wp2n, carParkIdArrival, blacklist,
						pickUpSegmentStr, StopPointRS, mergedWP);
//
//				std::cout << "origin->destination->carPark: WPs after merging:" << std::endl;
//				for (int i=0; i<mergedWP.size(); i++) {
//					std::cout << " -> " << mergedWP[i].roadSegment_->originalDB_ID.getLogItem();
//				}
//				std::cout << std::endl;
			} else if ((carParkNodeDeparture != originNode)
					&& (carParkNodeArrival != destNode)) { //find route from carPark to origin, origin to dest and from dest to the carPark and merge them
				std::vector < sim_mob::WayPoint > wp4 = getShortestPath(
						carParkIdDeparture, originNodeId);
				std::vector < sim_mob::WayPoint > mergedWP_temp;
				vector<const sim_mob::RoadSegment*> blacklist;

				calculateThePath(wp4, destNodeId, blacklist, pickUpSegmentStr, StopPointRS, mergedWP_temp);

				if (mergedWP_temp.size() > 0) { // or if StopPointRS->getStart() != originNode
					vector<const sim_mob::RoadSegment*> blacklist2;
					calculateThePath(mergedWP_temp, carParkIdArrival, blacklist2, dropOffSegmentStr, dropOffPointRS, mergedWP);
				} else {
					// reject the trip for now. There should be a path from different carPark, what should be fixed
					itr = serviceBuffer.erase(itr);
					continue;

				}

//				std::cout << "carpark->origin->destination->carPark: WPs after merging:" << std::endl;

			}else {
				// find route from origin to destination, as carpark1 = origin, carpark2 = dest
				for (int i=0; i<wp2.size(); i++) {
					if (wp2[i].type == WayPoint::ROAD_SEGMENT ) {
						mergedWP.push_back(wp2[i]);

						//add drop off location
						WayPoint lastWP = mergedWP[mergedWP.size()-1];
						const RoadSegment *lastWPrs = lastWP.roadSegment;
						std::stringstream segmentId("");
						segmentId << lastWPrs->getRoadSegmentId();
						pickUpSegmentStr = segmentId.str();
						StopPointRS = lastWPrs; //this is the segment where picking up f the passenger will occur
//
//						std::cout << "origin->destination: WPs after merging:" << std::endl;
//						for (int i=0; i<mergedWP.size(); i++) {
//							std::cout << " -> " << mergedWP[i].roadSegment_->originalDB_ID.getLogItem();
//						}
//						std::cout << std::endl;
					}
				}
			}

			if (mergedWP.size() <=0) {
				std::cout << "Merged path is of zero size! Not servicing" << std::endl;
				itr = serviceBuffer.erase(itr);
				continue;
			}

			//----------------------------------------------------------------------------------------
			//Create a AMOD Vehicle

			//remove the vehicle from the car park
			if (!removeVhFromCarPark(carParkIdDeparture, &vhAssigned)) {
				Print() << "Error! Cannot remove car from car park!" << std::endl;
				itr++;
				continue;
			}

			// create trip chain
			DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue() + startTime);

			sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", carParkNodeDeparture, "node", destNode, "node");
			SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), carParkNodeDeparture, "node", destNode, "node", "Car");
			((Trip*)tc)->addSubTrip(subTrip);

			std::vector<sim_mob::TripChainItem*>  tcs;
			tcs.push_back(tc);
			vhAssigned->setTripChain(tcs); //add trip chain
			vhAssigned->amdoTripId = tripId;
			vhAssigned->setPath(mergedWP);

			//amod pickUpSegment for the dwell time implementation
			if (StopPointRS != NULL){
				vhAssigned->amodSegmLength = StopPointRS->getLength();
				vhAssigned->amodPickUpSegmentStr = pickUpSegmentStr;
			}
			else {
				vhAssigned->amodSegmLength = 0.0;
				vhAssigned->amodPickUpSegmentStr = "-1";
			}

			if (dropOffPointRS != NULL){
				vhAssigned->amodSegmLength2 = dropOffPointRS->getLength();
				vhAssigned->amodDropOffSegmentStr = dropOffSegmentStr;
			}
			else {
				vhAssigned->amodSegmLength2 = 0.0;
				vhAssigned->amodDropOffSegmentStr = "-1";
			}


			// set event
			eventPub.registerEvent(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH);
			eventPub.subscribe(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH, vhAssigned, &Person::handleAMODEvent, vhAssigned);

			//----------------------------------------------------------------------------------------
			//create a trip map to keep track of vehicles

			AmodTrip atrip = *itr;
			atrip.carParkIdDeparture = carParkIdDeparture;
			atrip.carParkIdArrival = carParkIdArrival;
			atrip.assignedAmodId = vhAssigned->amodId;
			atrip.dispatchTime = currTime;
			atrip.pickUpTime = -1;
			atrip.arrivalTime = -1;
			atrip.tripError = false;
			atrip.finalSegment = mergedWP[mergedWP.size()-1].roadSegment->getRoadSegmentId();

			if (carParkNodeDeparture == originNode) {
				atrip.pickedUp = true;
				atrip.pickUpSegment = mergedWP[0].roadSegment->getRoadSegmentId();
				atrip.pickUpTime = atrip.dispatchTime;
			} else {
				atrip.pickedUp = false;
				atrip.pickUpSegment = wp2[0].roadSegment->getRoadSegmentId();
			}


			double tripDistance = 0;
			WayPoint iterWP;
			const RoadSegment *iterwp;
			//Print() << "Length of segments: ";
			for (int j = 0; j< mergedWP.size(); j++){
				iterWP = mergedWP[j];
				const RoadSegment *rs = iterWP.roadSegment;
				tripDistance += mergedWP[j].roadSegment->getLength();
			}
			atrip.tripDistanceInM = tripDistance/100;

			//----------------------------------------------------------------------------------------
			// dispatch vehicle
			dispatchVh(vhAssigned);
			justDispatched.push_back(vhAssigned);
			TripMapIterator tripItr;
			vhTripMapMutex.lock();
			tripItr = vhTripMap.insert(std::make_pair(vhAssigned, atrip)).first;
			tripItr->second = atrip; //because the first line doesn't replace.
			vhTripMapMutex.unlock();

			// output
			//out_demandStat << tripId << " " << originNodeId << " " << destNodeId << " " << reqTime << " servicing " << "accepted " << " timePickedUp? " << "timeDropped? " << tripDistanceInM << "\n";
			//out_vhsStat << vhAssigned->amodId << " " << currTime << " posX " << "posY " << "inCarpark " << carParkId << " onRoad " << "\n";

			//pop out the serviced client
			itr = serviceBuffer.erase(itr);
		} //end while loop

		//add just dispatched vehicles to the road
		for (int i=0; i<justDispatched.size(); i++) {
			vhOnTheRoad.insert(std::make_pair(justDispatched[i]->amodId,justDispatched[i]));
		}

		Print() << " # of AMOD vehicles onroad: " << vhOnTheRoad.size()<< std::endl;
		Print() << " # of AMOD vehicles parked: " << nFreeCars << std::endl;
		std::cout << " # of empty vehicles on the road: " << EmptyVhOnTheRoad.size()<< std::endl;

	}
	else
	{
		Print() << "Unable to open out_vhsStat or out_demandStat";
	}

}

void AMODController::rebalanceVhs()
{ //given number of vehicles at each carPark, redistribute vehicles to have equal # of vhs at each carpark

	//check how many vhs at each carpark
	//interate through all the carparks
	vector<int> numOfVhsVec; // this vector also stores # of vhs at each node from the carParksIds vector
	for(int i = 0; i < carParkIds.size(); i++){
		EmptyVhTrip etrip;
		string carParkId = carParkIds[i];
		AMODVirtualCarParkItor iter;
		//	boost::unordered_map<std::string,int> numberOfVhsMap; // to store # of vhs at each node
		int numOfVhs =0;
		//iterate through all vehicles at the carpark
		for (iter=virtualCarPark.begin(); iter != virtualCarPark.end(); iter++) {
			string ids = iter->first; // not sure now how this string looks like, maybe I need to take a number from this string
			if(ids == carParkId){
				numOfVhs++;
			}
		}
		numOfVhsVec.push_back(numOfVhs);
		//numberOfVhsMap.insert(std::make_pair(carParkId, numOfVhs));
	}
	// pass matrix of distances between all carparks to julia
	std::vector<std::vector < double > > matrixOfDist = distancesBetweenCarparks;

	// pass numberOfVhsVec to julia

	// bring from julia matrix of how many vhs should be send from i to j
	std::vector<std::vector < double > > numOfVhsToBeRedistributed;

	// assign vehicles to empty trips
	// empty trips from each carPark to all other carParks

	for (int k =0; k< numOfVhsToBeRedistributed.size(); k++)
	{
		vector < double > vhsToGo = numOfVhsToBeRedistributed[k];
		for (int kk = 0; kk< vhsToGo.size(); kk++)
		{
			double numVhs = vhsToGo[kk];
			for(int kkk = 0; kkk < numVhs; kkk++){
				// generate OD and dispatch the veh, origin-> k element in carParks, destination is kk element
				string origin = carParkIds[k];
				string destination = carParkIds[kk];

				//assignVhToEmptyTrip(origin, destination, currTime);
			}
		}
	}


}

void assignVhToEmptyTrip(std::string& origin, std::string& destination, int currTime)
{
	//	Person* vhAssigned=nullptr;
	//	int startTime = currTime;
	//	// calculate sp
	//	vector<WayPoint> sp = getShortestPath(origin, destination);
	//	if ((sp.size() == 0) && origin !=destination) {
	//		std::cout << " Rebalancing: rejected " << "no_path_from_carpark1 " << origin << " to carpark2 " << destination << std::endl;
	//		continue;
	//	}else
	//	{ //dispach vh
	//		//Create a AMOD Vehicle
	//		//remove the vehicle from the car park
	//		if (!removeVhFromCarPark(origin, &vhAssigned)) {
	//			std::cout << "Rebalancing: Error! Cannot remove car from car park!" << std::endl;
	//			continue;
	//		}
	//
	//		// create trip chain
	//		DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue() + startTime);
	//		//			DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+ConfigManager::GetInstance().FullConfig().baseGranMS());
	//		sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", origin, "node", destination, "node");
	//		SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), origin, "node", destination, "node", "Car");
	//		((Trip*)tc)->addSubTrip(subTrip);
	//
	//		std::vector<sim_mob::TripChainItem*>  tcs;
	//		tcs.push_back(tc);
	//		vhAssigned->setTripChain(tcs); //add trip chain
	//
	//		std::cout << "Assigned Path: Empty vh ride: ";
	//		for (int i=0; i<sp.size(); i++) {
	//			std::cout << " -> " << sp[i].roadSegment_->originalDB_ID.getLogItem();
	//		}
	//		std::cout << std::endl;
	//
	//		vhAssigned->setPath(sp);
	//
	//	}
	//
	//	// dispatch vehicle
	//	dispatchVh(vhAssigned);
	//	EmptyVhOnTheRoad.insert(std::make_pair(vhAssigned->amodId,vhAssigned));
	//
	//	double tripDistance;
	//	WayPoint iterWP;
	//	const RoadSegment *iterwp;
	//	for (int j = 0; j< sp.size(); j++){
	//		iterWP = sp[j];
	//		const RoadSegment *rs = iterWP.roadSegment_;
	//		tripDistance += sp[j].roadSegment_->length;
	//	}

}

void AMODController::frame_output(timeslice now)
{

}

} /* namespace AMOD */
} /* namespace sim_mob */
