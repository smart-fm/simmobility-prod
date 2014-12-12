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
#include "geospatial/Link.hpp"
#include "geospatial/PathSetManager.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/UniNode.hpp"
#include "logging/Log.hpp"
#include "metrics/Frame.hpp"
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
	stdir = &StreetDirectory::instance();

	const sim_mob::RoadNetwork& roadNetwork = ConfigManager::GetInstance().FullConfig().getNetwork();
	const std::vector<sim_mob::MultiNode*>& multiNodesPool = roadNetwork.getNodes();
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

	for(std::vector<sim_mob::MultiNode *>::const_iterator itMultiNodes = multiNodesPool.begin(); itMultiNodes != multiNodesPool.end(); ++itMultiNodes)
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

			std::vector < WayPoint > wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*(origItr->second)), stdir->DrivingVertex(*(destItr->second)),blacklist);
			for (int i=0; i<wp2.size(); i++) {
				if (wp2[i].type_ == WayPoint::ROAD_SEGMENT ) {
					wp.push_back(wp2[i]);
				}
			}

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
		std::vector<const sim_mob::RoadSegment*> blacklist;

		std::vector<WayPoint> wp2 = stdir->SearchShortestDrivingPath(
				stdir->DrivingVertex(*origNode),
				stdir->DrivingVertex(*destNode), blacklist);

		for (std::vector<WayPoint>::iterator itWayPts = wp2.begin(); itWayPts != wp2.end(); ++itWayPts)
		{
			if ((*itWayPts).type_ == WayPoint::ROAD_SEGMENT)
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

		std::vector < WayPoint > wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*origNode), stdir->DrivingVertex(*destNode),blacklist);
		for (int i=0; i<wp2.size(); i++) {

			if (wp2[i].type_ == WayPoint::ROAD_SEGMENT ) {
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

	const Node* origNode = nodePool.at(origNodeID);
	const Node* destNode = nodePool.at(destNodeID);

	if (origNode == destNode) {
		return wp;
	}
	// compute shortest path
	std::vector < WayPoint > wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*origNode), stdir->DrivingVertex(*destNode),blacklist);
	for (int i=0; i<wp2.size(); i++) {
		if (wp2[i].type_ == WayPoint::ROAD_SEGMENT ) {
			wp.push_back(wp2[i]);
		}
	}

	return wp;
}


Entity::UpdateStatus AMODController::frame_tick(timeslice now)
{
	int current_time = now.ms();
	currTime = current_time;
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
		>> vehStatOutputModulus;

		myFile.open(demandFileName.c_str());
		carParkFile.open(carParkFileName.c_str());
		out_demandStat.open(outDemandFileName.c_str());
		out_vhsStat.open(outVhsStatFilename.c_str());
		out_tripStat.open(outTripStatFilename.c_str());

		populateCarParks(nCarsPerCarPark);

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
	std::vector<std::string> carParkIds;

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


bool AMODController::getBestFreeVehicle(std::string originId, sim_mob::Person **vh, std::string &carParkId, std::vector < sim_mob::WayPoint > &leastCostPath, double &bestTravelCost) {

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
		carParkId = bestCarParkIter->first;
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
		if(wp[i].type_ == WayPoint::ROAD_SEGMENT )
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
	double oX = originNode->location.getX();
	double oY = originNode->location.getY();

	std::vector< distPair > nodeDistances;

	// loop through nodePool
	typedef std::map<std::string, sim_mob::Node*>::iterator it_type;
	for(it_type iterator = nodePool.begin(); iterator != nodePool.end(); iterator++) {
		std::string nodeId = iterator->first;
		Node *tempNode = iterator->second;

		double tX = tempNode->location.getX();
		double tY = tempNode->location.getY();

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
		if (carparkToOrigin[i].type_ == WayPoint::ROAD_SEGMENT ) {
			mergedWP.push_back(carparkToOrigin[i]);
		}
	}

	for (int i=0; i<originToDestination.size(); i++) {
		if (originToDestination[i].type_ == WayPoint::ROAD_SEGMENT )  {
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

			tt.startTime = (simStart + sim_mob::DailyTime(frameNumber.ms())).toString();
			double frameLength = ConfigManager::GetInstance().FullConfig().baseGranMS();
			tt.endTime = (simStart + sim_mob::DailyTime(frameNumber.ms() + frameLength)).toString();
			tt.travelTime = (*it).second.travelTimeSum/(*it).second.agCnt;

			PathSetManager::getInstance()->insertTravelTime2TmpTable(tt);
		}
	}
	return res;
}

void AMODController::testTravelTimePath()
{
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
		if(wp[i].type_ == WayPoint::ROAD_SEGMENT )
		{
			const sim_mob::RoadSegment* rs = wp[i].roadSegment_;
			Print() <<"from node: "<<rs->getStart()->originalDB_ID.getLogItem()<<" to node: "<<rs->getEnd()->originalDB_ID.getLogItem()<<std::endl;
		}
	}
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
		if ((*wPIter).type_ == WayPoint::ROAD_SEGMENT) {
			const RoadSegment *rs = (*wPIter).roadSegment_;
			travelTime += rs->getLengthOfSegment()/(rs->maxSpeed*27.778);
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
				<< a.destination << " " << a.carParkId << " "
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
			if (!hasShortestPath(origin[i], destination[i])){
				if (origin[i] == destination[i]) {
					out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "0\n"; //origin same as destination
				} else {
					out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "1\n"; //no path to destination
				}
				continue; //not possible to service this trip. no possible route.
			}

			AMODVirtualCarParkItor iter;
			bool validDemand = false;

			//check if there is route from a car park to the origin
			for (iter=virtualCarPark.begin(); iter != virtualCarPark.end(); iter++) {
				if (hasShortestPath(iter->first, origin[i])) {
					validDemand = true;
					break;
				}
			}

			if (!validDemand) {
				out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "2" << std::endl;
				continue;
			}

			//check if there is a route from the destination to a car park
			validDemand = false;
			//check if there is route from a car park to the destinattion
			for (iter=virtualCarPark.begin(); iter != virtualCarPark.end(); iter++) {
				if (hasShortestPath(destination[i], iter->first)) {
					validDemand = true;
					break;
				}
			}
			if (!validDemand) {
				out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 0 " << "3" << std::endl;
				continue;
			}

			//is a valid trip
			out_demandStat << tripID[i] << " " << atrip.requestTime << " " << origin[i] << " " << destination[i] << " 1 " << "0" << std::endl;

			serviceBuffer.push_back(atrip);
		}

		Print() << "AMOD Service Buffer Size: " << serviceBuffer.size()
				<< ", Free Cars: " << nFreeCars
				<< std::endl;

		// work through list using available free cars
		ServiceIterator itr =serviceBuffer.begin();
		int startTime = currTime;
		const RoadSegment *StopPointRS = nullptr; //vh will stop at this segment to pick up passenger
		string pickUpSegmentStr;

		while (true) {
			if (nFreeCars <= 0) break; //check if the number of free cars is non-zero
			if (serviceBuffer.size() == 0) break;
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
			std::string carParkId;
			bool freeVehFound = getBestFreeVehicle(originNodeId, &vhAssigned, carParkId, leastCostPath, bestFreeVehTravelCost);
			if (!freeVehFound)
			{
				if (carParkId == originNodeId) {
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
						out_demandStat << tripId << " " << reqTime << " " << originNodeId << " " << destNodeId << " " << reqTime << " 0 " << "2 "
								<< carParkId << std::endl;
						itr = serviceBuffer.erase(itr);
					}
				}
				continue;
			}

			//----------------------------------------------------------------------------------------
			// Compute route for found AMOD vehicle
			const Node *carParkNode = getNodeFrmPool(carParkId);
			if(carParkNode==nullptr){
				Print()<<"some error should be happen in AmodController(carParkNode)"<< std::endl;
				return;
			}

			// get route for vehicle
			vector<WayPoint> mergedWP;
			std::vector<WayPoint> wp2 = getShortestPath(originNodeId, destNodeId); //shortest path sed in case when the carpark is at the same node as the origin

			//check to see if there is a route from the destination back to the carpark
			//if not, then this is a problem since we cannot get back into the network
			std::vector<WayPoint> wp3 = getShortestPath(destNodeId, carParkId);
			if (wp3.empty()) {
				out_demandStat << tripId << " " << reqTime << " " << originNodeId << " " << destNodeId << " " << reqTime << " 0 " << "3 "
						<< carParkId << std::endl;
				itr = serviceBuffer.erase(itr);
				continue;
			}

			if (carParkNode != originNode) {
				std::vector<WayPoint> wp1 = getShortestPath(carParkId, originNodeId);

				//get the last WayPoint from the wp1
				WayPoint lastWP = wp1[wp1.size()-1];
				const RoadSegment *lastWPrs = lastWP.roadSegment_;
				pickUpSegmentStr = lastWPrs->originalDB_ID.getLogItem();

				StopPointRS = lastWP.roadSegment_; //this is the segment where picking up f the passenger will occur

				//erase the last WayPoint from the wp1
				wp1.pop_back();

				//get the second last node as a new start node for the second part of the trip
				const Node* startNode = lastWPrs->getStart();

				//check if multi node
				const MultiNode* currEndNode = dynamic_cast<const MultiNode*>(startNode);
				std::vector<const sim_mob::RoadSegment*> blacklist;
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

					for(int i = 0; i < allSeg.size(); ++i){
						const RoadSegment* rs = allSeg[i];
						if(rs != lastWPrs){
							blacklist.push_back(rs);
						}
					}
				}
				//calculate sp with blacklist
				std::string s1 = startNode->originalDB_ID.getLogItem();
				string startNodeId = getNumberFromAimsunId(s1);

				wp2 = getShortestPathWBlacklist(startNodeId, destNodeId, blacklist);

				//merge wayPoints
				mergeWayPoints(wp1, wp2, mergedWP);
			} else {
				// find route from origin to destination
				for (int i = 0; i < wp2.size(); i++) {
					if (wp2[i].type_ == WayPoint::ROAD_SEGMENT ) {
						mergedWP.push_back(wp2[i]);
					}
				}
			}

			if (mergedWP.empty()) {
				Print() << "Merged path is of zero size! Not servicing" << std::endl;
				itr = serviceBuffer.erase(itr);
				continue;
			}

			//----------------------------------------------------------------------------------------
			//Create a AMOD Vehicle

			//remove the vehicle from the car park
			if (!removeVhFromCarPark(carParkId, &vhAssigned)) {
				Print() << "Error! Cannot remove car from car park!" << std::endl;
				itr++;
				continue;
			}

			// create trip chain
			DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue() + startTime);

			sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", carParkNode, "node", destNode, "node");
			SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), carParkNode, "node", destNode, "node", "Car");
			((Trip*)tc)->addSubTrip(subTrip);

			std::vector<sim_mob::TripChainItem*>  tcs;
			tcs.push_back(tc);
			vhAssigned->setTripChain(tcs); //add trip chain
			vhAssigned->amdoTripId = tripId;
			vhAssigned->setPath(mergedWP);

			//amod pickUpSegment for the dwell time implementation
			if (StopPointRS != NULL) {
				vhAssigned->amodSegmLength = StopPointRS->length;
				vhAssigned->amodPickUpSegmentStr = pickUpSegmentStr;
			} else {
				vhAssigned->amodSegmLength = 0.0;
				vhAssigned->amodPickUpSegmentStr = "-1";
			}

			// set event
			eventPub.registerEvent(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH);
			eventPub.subscribe(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH, vhAssigned, &Person::handleAMODEvent, vhAssigned);

			//----------------------------------------------------------------------------------------
			//create a trip map to keep track of vehicles

			AmodTrip atrip = *itr;
			atrip.carParkId = carParkId;
			atrip.assignedAmodId = vhAssigned->amodId;
			atrip.dispatchTime = currTime;
			atrip.pickUpTime = -1;
			atrip.arrivalTime = -1;
			atrip.tripError = false;
			atrip.finalSegment = mergedWP[mergedWP.size()-1].roadSegment_->getId();
			vhAssigned->stuckCount = 0;
			vhAssigned->prevx = 0;
			vhAssigned->prevy = 0;

			if (carParkNode == originNode) {
				atrip.pickedUp = true;
				atrip.pickUpSegment = mergedWP[0].roadSegment_->getId();
				atrip.pickUpTime = atrip.dispatchTime;
			} else {
				atrip.pickedUp = false;
				atrip.pickUpSegment = wp2[0].roadSegment_->getId();
			}


			double tripDistance = 0;
			WayPoint iterWP;
			const RoadSegment *iterwp;
			Print() << "Length of segments: ";
			for (int j = 0; j< mergedWP.size(); j++){
				iterWP = mergedWP[j];
				const RoadSegment *rs = iterWP.roadSegment_;
				tripDistance += mergedWP[j].roadSegment_->length;
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

	}
	else
	{
		Print() << "Unable to open out_vhsStat or out_demandStat";
	}

}



void AMODController::frame_output(timeslice now)
{

}

} /* namespace AMOD */
} /* namespace sim_mob */
