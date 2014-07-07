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
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/UniNode.hpp"
#include "geospatial/PathSetManager.hpp"
#include "entities/misc/TripChain.hpp"
#include "workers/Worker.hpp"
#include "metrics/Frame.hpp"
#include <utility>
#include <stdexcept>
#include <string>
#include <fstream>
#include <algorithm>
#include "entities/vehicle/Vehicle.hpp"
#include "entities/roles/Role.hpp"
#include <limits>
#include <sstream>

using namespace std;

namespace sim_mob {
namespace AMOD
{
AMODController* AMODController::pInstance = nullptr;
//boost::asio::io_service AMODController::ioService;

sim_mob::AMOD::AMODController::~AMODController() {
	// TODO Auto-generated destructor stub
}
void sim_mob::AMOD::AMODController::init()
{
	stdir = &StreetDirectory::instance();

	const sim_mob::RoadNetwork* roadNetwork = &ConfigManager::GetInstance().FullConfig().getNetwork();
	const std::vector<sim_mob::MultiNode*> multiNodesPool = roadNetwork->getNodes();
	const std::set<sim_mob::UniNode*> uniNodesPool = roadNetwork->getUniNodes();

	for(std::vector<sim_mob::Link *>::const_iterator it = roadNetwork->getLinks().begin(), it_end(roadNetwork->getLinks().end()); it != it_end ; it ++)
	{
		for(std::set<sim_mob::RoadSegment *>::iterator seg_it = (*it)->getUniqueSegments().begin(), it_end((*it)->getUniqueSegments().end()); seg_it != it_end; seg_it++)
		{
			if (!(*seg_it)->originalDB_ID.getLogItem().empty())
			{
				string aimsunId = (*seg_it)->originalDB_ID.getLogItem();
				string segId = getNumberFromAimsunId(aimsunId);
				//				Print()<<aimsun_id<<std::endl;
				segPool.insert(std::make_pair(segId,*seg_it));
			}
		}
	}
	//
	for(int i=0;i<multiNodesPool.size();++i)
	{
		sim_mob::Node* n = multiNodesPool.at(i);
		if (!n->originalDB_ID.getLogItem().empty())
		{
			std::string aimsunId = n->originalDB_ID.getLogItem();
			std::string id = getNumberFromAimsunId(aimsunId);
			nodePool.insert(std::make_pair(id,n));
		}
	}
	for(std::set<sim_mob::UniNode*>::iterator it=uniNodesPool.begin(); it!=uniNodesPool.end(); ++it)
	{
		sim_mob::UniNode* n = (*it);
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
	//boost::unordered_map<std::string, vector <WayPoint> > shortestPaths;
	std::map<std::string,sim_mob::Node*>::iterator origItr;
	std::map<std::string,sim_mob::Node*>::iterator destItr;
	std::cout << "Pre-computing shortest paths" << std::endl;
	for (origItr = nodePool.begin(); origItr != nodePool.end(); origItr++) {
		std::cout << "Node: " << origItr->first << std::endl;
		for (destItr = nodePool.begin(); destItr != nodePool.end(); destItr++) {
			std::string nodeKey;

			nodeKey = string(origItr->first) + "-" + string(destItr->first);
			std::vector < WayPoint > wp;
			if (origItr->second == destItr->second) {
				shortestPaths.insert(std::make_pair(nodeKey, wp));
				continue;
			}

			// compute shortest path
			const sim_mob::RoadSegment* exclude_seg;
			std::vector<const sim_mob::RoadSegment*> blacklist;
			if(exclude_seg)
			{
				blacklist.push_back(exclude_seg);
			}

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

vector <WayPoint> AMODController::getShortestPath(std::string origNodeID, std::string destNodeID)
{
	std::string nodeKey = origNodeID + "-" + destNodeID;
	boost::unordered_map<std::string, vector < WayPoint > >::iterator spItr = shortestPaths.find(nodeKey);
	if (spItr == shortestPaths.end()) {
		//std::cout << "No such node pair. On-the-fly computation" << std::endl;
		std::vector < WayPoint > wp;

		Node* origNode = nodePool[origNodeID];
		Node* destNode = nodePool[destNodeID];

		if (origNode == destNode) {
			shortestPaths.insert(std::make_pair(nodeKey, wp));
			return wp;
		}

		// compute shortest path
		const sim_mob::RoadSegment* exclude_seg;
		std::vector<const sim_mob::RoadSegment*> blacklist;
		if(exclude_seg)
		{
			blacklist.push_back(exclude_seg);
		}

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


Entity::UpdateStatus AMODController::frame_tick(timeslice now)
{
	//std::cout<< "Time:" << now.ms() << std::endl;

	if(test==0)
	{

		populateCarParks(2);
		myFile.open("/home/km/dataFiles/readDemandFile/bugisSmall_withID.txt");
		out_demandStat.open("/home/km/dataFiles/outputFiles/out_demandStat.txt");
		out_vhsStat.open("/home/km/dataFiles/outputFiles/out_vhsStat.txt");
		//myFile.open("/home/haroldsoh/Development/simmobility/dataFiles/inputfile1.txt");
		lastReadLine = "";

		if (out_demandStat.is_open()) {
			out_demandStat << "tripID " << "origin " << "destination " << "timeRequested " << "accepted/rejected " << "reasonRejected " << "timePickedUp " << "TimeDropped " << "tripDistance\n" ;
		}
		else cout << "Unable to open out_demandStat.txt";
		//demandDiary.close();

		if (out_vhsStat.is_open()) {
			out_vhsStat << "vehId " << "time " << "pos(X) " << "pos(Y) " << "onRoad " << "segmentId" << " nodeId " << "inCarpark\n" ;
		}
		else cout << "Unable to open out_vhsStat.txt";

		test=1;
	}

	if(test==1)
	{

		boost::unordered_map<std::string,Person*>::iterator itr;
		for (itr = vhOnTheRoad.begin(); itr != vhOnTheRoad.end(); itr++) {
			std::string amodId = itr->first;
			Person *vh = itr->second;
			if (vh->amodVehicle != NULL) {
				//	std::cout << amodId << " segment: " << vh->amodVehicle->getCurrSegment()->getSegmentID() << std::endl;
				//	std::cout << amodId << " lane: " << vh->amodVehicle->getCurrLane()->getLaneID() << std::endl;
			}
		}


		//reading the file of time, destination and origin
		int current_time = now.ms();
		std::cout << "current_time in frame_tick: " << current_time << std::endl;
		std::vector<string> origin;
		std::vector<string> destination;
		std::vector<string> tripID;
		std::cout << "Reading the demand file... " << std::endl;

		readDemandFile(tripID, current_time, origin, destination);

		saveVehStat(current_time);

		//assignVhs(origin, destination);
		assignVhsFast(tripID, origin, destination, current_time);
		test = 1;

		if (now.ms()==119900){
			out_demandStat.close();
			out_vhsStat.close();
		}
	}
	// return continue, make sure agent not remove from main loop
	return Entity::UpdateStatus::Continue;


}

void AMODController::populateCarParks(int numberOfVhsAtNode = 10)
{
	//carpark population
	std::vector<std::string> carParkIds;
	ifstream carParkFile("/home/km/dataFiles/populateCarParks/bugisSmall.txt");
	while(!carParkFile.eof())
	{
		std::string line;
		std::getline (carParkFile,line);
		if (line == "") break;

		carParkIds.push_back(line);
	}

	std::cout << "CarparkIds: " << std::endl;
	copy(carParkIds.begin(), carParkIds.end(), ostream_iterator<string>(cout, " "));
	std::cout << endl << "---------------" << std::endl;

	int k = 0;
	std::vector<Person*> vhs;
	//while (!carParkIds.empty())
	for(int j = 0; j<carParkIds.size(); j++)
	{
		for(int i = 0; i<numberOfVhsAtNode; i++)
		{
			std::string vhId = "amod-";
			string uId;          // string which will contain the result
			ostringstream convert;   // stream used for the conversion
			convert << k;      // insert the textual representation of 'Number' in the characters in the stream
			k++;
			uId = convert.str(); // set 'Result' to the contents of the stream
			vhId += uId;
			std::cout << vhId << std::endl;

			std::string carParkId = carParkIds[j];
			std::cout << "Inserting to the caprark: " << carParkId << std::endl;
			addNewVh2CarPark(vhId,carParkId);

		}
		//carParkIds.pop_back();
		//std::cout << "Cars inserted. Left carparkIds: " << std::endl;
		//copy(carParkIds.begin(), carParkIds.end(), ostream_iterator<string>(cout, " "));
		//std::cout << endl << "---------------" << std::endl;
	}
}

void AMODController::readDemandFile(vector<string>& tripID, int current_time, vector<string>& origin, vector<string>& destination)
{
	string line;
	string tripID_;
	string time_;
	string origin_;
	string destination_;
	//ifstream myfile; //("/home/km/Dropbox/research/autonomous/automated-MoD/simMobility_implementation/txtFiles/About10.txt");
	std::cout << "current_time inside the readDemandFile: " << current_time << std::endl;

	if (myFile.is_open())
	{
		std::cout << "file is open. I am inside the readDemandFile function." << std::endl;

		while(!myFile.eof())
		{
			if(lastReadLine =="")
			{
				std::getline (myFile,line);
				std::cout << "getting line" <<std::endl;
			}
			else{
				line = lastReadLine;
				lastReadLine = "";
			}
			if (line == "") break;

			std::stringstream ss;
			ss << line;
			ss >> tripID_ >> time_ >> origin_ >> destination_;
			std::cout << "myFile >> tripID_: " << tripID_ << ">> time_:" << time_ << " >> origin_: " << origin_ << " >> destination_: " << destination_ << std::endl;
			int time = atoi(time_.c_str());

			if ((time*1000) <= current_time)
			{
				tripID.push_back(tripID_);
				origin.push_back(origin_);
				destination.push_back(destination_);
				std::cout << "origin.push_back(origin_) " << std::endl;
				copy(origin.begin(), origin.end(), ostream_iterator<string>(cout, " "));
				std::cout << std::endl;
				lastReadLine = "";
			} else {
				lastReadLine = line;
				break;
			}

		}
		//myFile.close();
	}
	else {
		cout << "Unable to open file" << endl;
	}

	//origin.pop_back();
	//destination.pop_back();

	//cout << "OD Pairs"<< endl;
	//std::cout << myFile << std::endl;
	//	std::cout << "Origin: " << std::endl;
	//	copy(origin.begin(), origin.end(), ostream_iterator<string>(cout, " "));
	//	std::cout << endl << "---------------" << std::endl;
	//	std::cout << "Destination: " << std::endl;
	//	copy(destination.begin(), destination.end(), ostream_iterator<string>(cout, " "));
	//	std::cout << endl << "---------------" << std::endl;
	//	std::cout << "Time: " << time_ << std::endl;
	//	std::cout << endl << "---------------" << std::endl;
}

void AMODController::addNewVh2CarPark(std::string& id,std::string& nodeId)
{
	// find node
	Node* node = nodePool[nodeId];

	if(node == NULL){ throw std::runtime_error("node not found"); }

	// create person
	DailyTime start = ConfigManager::GetInstance().FullConfig().simStartTime(); // DailyTime b("08:30:00");
	sim_mob::Trip* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", node, "node", node, "node");
	sim_mob::SubTrip subTrip("", "Trip", 0, 1, start, DailyTime(), node, "node", node, "node", "Car");
	tc->addSubTrip(subTrip);
	std::vector<sim_mob::TripChainItem*>  tcs;
	tcs.push_back(tc);

	//std::cout<<ConfigManager::GetInstance().FullConfig().simStartTime().getValue()<<std::endl;
	sim_mob::Person* person = new sim_mob::Person("AMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
	//person->setTripChain(tcs);
	//std::cout<<"starttime: "<<person->getStartTime()<<std::endl;
	person->parentEntity = this;
	person->amodId = id;

	// add to virtual car park
	AMODVirtualCarParkItor it = virtualCarPark.find(nodeId);
	if(it!=virtualCarPark.end())
	{
		// access this car park before
		std::cout << "Existing car park" << std::endl;
		boost::unordered_map<std::string,Person*> cars = it->second;
		std::cout << "Before Insertion. Cars Size: " << cars.size() << std::endl;
		cars.insert(std::make_pair(id,person));
		std::cout << "Inserted. Cars Size: " << cars.size() << std::endl;

		boost::unordered_map<std::string,Person*>::iterator local_it;
		std::cout << "Cars in Car Park : \n";
		for ( local_it = cars.begin(); local_it!= cars.end(); ++local_it ) {
			std::cout << " " << local_it->first << ":" << local_it->second << std::endl;
		}
		std::cout << "-----\n";

		it->second = cars;
	}
	else
	{
		std::cout << "New car park" << std::endl;
		boost::unordered_map<std::string,Person*> cars = boost::unordered_map<std::string,Person*>();
		cars.insert(std::make_pair(id,person));
		virtualCarPark.insert(std::make_pair(nodeId,cars));
		std::cout << "Inserted. Cars Size: " << cars.size() << std::endl;
	}

	//insert this car into the global map of all cars in car parks
	vhInCarPark.insert( std::make_pair(id, person) );
	allAMODCars.insert( std::make_pair(id, person) ); //add this car to the global map of all AMOD cars (for bookkeeping)
	nFreeCars++;
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
		*vh = firstCarIt->second;
	}
	return freeCarFound;
}

double AMODController::getTravelTimePath(std::string startNodeId, std::string endNodeId, vector < sim_mob::WayPoint > &leastCostPath) {

	sim_mob::Node* startNode;
	sim_mob::Node* endNode;
	startNode = nodePool[startNodeId];
	endNode = nodePool[endNodeId];

	cout << "start node: " << startNodeId << ", end node: " << endNodeId << endl;

	const sim_mob::RoadSegment* exclude_seg;
	std::vector<const sim_mob::RoadSegment*> blacklist;
	if(exclude_seg)
	{
		blacklist.push_back(exclude_seg);
	}

	//std::vector<WayPoint> wp = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*startNode), stdir->DrivingVertex(*endNode),blacklist);

	std::vector<WayPoint> wp = getShortestPath(startNodeId, endNodeId);

	for(int i=0;i<wp.size();++i)
	{
		if(wp[i].type_ == WayPoint::ROAD_SEGMENT )
		{
			//std::cout << "Wp: " << i << wp[i].roadSegment_->getSegmentID() << std::endl;
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
	AMODVirtualCarParkItor it = virtualCarPark.find(carParkId);
	if(it==virtualCarPark.end()){

		//throw std::runtime_error("no this car park...");
		return false;
	}

	boost::unordered_map<std::string,Person*> cars = it->second;
	if(!cars.empty())
	{
		boost::unordered_map<std::string,Person*>::iterator firstCarIt = cars.begin();
		*vh = firstCarIt->second;
		cars.erase(firstCarIt);
		it->second = cars;
		std::cout << "Cars size: " << cars.size() << std::endl;
		vhInCarPark.erase( (*vh)->amodId );
		(*vh)->currStatus = Person::ON_THE_ROAD;
		nFreeCars--;
		return true;
	}

	return false;
}

bool AMODController::removeVhFromCarPark(std::string& carParkId,Person** vh)
{
	AMODVirtualCarParkItor it = virtualCarPark.find(carParkId);
	if(it==virtualCarPark.end()){

		//throw std::runtime_error("no this car park...");
		return false;
	}

	boost::unordered_map<std::string,Person*> cars = it->second;
	if(!it->second.empty())
	{
		if (cars.find((*vh)->amodId) == cars.end()) {
			return false;
		}
		cars.erase((*vh)->amodId);
		//it->second.erase((*vh)->amodId);
		it->second = cars;
		vhInCarPark.erase( (*vh)->amodId );
		(*vh)->currStatus = Person::ON_THE_ROAD;
		nFreeCars--;
		return true;
	}

	return false;
}


void AMODController::mergeWayPoints(const std::vector<sim_mob::WayPoint>& carparkToOrigin, const std::vector<sim_mob::WayPoint> &originToDestination, std::vector<sim_mob::WayPoint>& mergedWP)
{
	//mergedWP.push_back(carparkToOrigin);
	//mergedWP.push_back(originToDestination);

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


	//mergedWP.reserve( carparkToOrigin.size() + originToDestination.size() ); // preallocate memory
	//mergedWP.insert( mergedWP.end(), carparkToOrigin.begin(), carparkToOrigin.end() );
	//mergedWP.insert( mergedWP.end(), originToDestination.begin(), originToDestination.end() );

}

bool AMODController::dispatchVh(Person* vh)
{
	this->currWorkerProvider->scheduleForBred(vh);
}


void AMODController::handleVHError(Person *vh)
{
	std::cout << vh->amodId << " suffered an error!" << std::endl;
	vhOnTheRoad.erase(vh->amodId);
	nFreeCars--;
	//find the name of the node where the car is supposed to go. We teleport it there
	TripMapIterator iter = vhTripMap.find(vh);
	if (iter == vhTripMap.end()) {
		std::cout << "ERROR! This should never happen!" << std::endl;
	}
	//add it to the car park there
	string idNode =iter->second.destination;

	addNewVh2CarPark(vh->amodId, idNode);

}

void AMODController::handleVHArrive(Person* vh)
{

	WayPoint w = vh->amodPath.back();
	const RoadSegment *rs = w.roadSegment_;
	const Node *enode = rs->getEnd();
	Node *pnode = (Node *) rs->getEnd();

	std::cout << vh->amodId << " arriving at " << pnode->nodeId << std::endl;
	//std::cout << "Check against: " << vh->amodVehicle->getCurrSegment()->getEnd()->nodeId << std::endl;

	std::string idNode = enode->originalDB_ID.getLogItem();// "aimsun-id":"123456"
	std::cout << "NodeId before: " << idNode << std::endl;

	char chars[] = "aimsun-id:,\"";
	for (unsigned int i = 0; i < strlen(chars); ++i)
	{
		idNode.erase (std::remove(idNode.begin(), idNode.end(), chars[i]), idNode.end());
	}
	std::cout << "NodeId after: " << idNode << std::endl;

	std::string vhID = vh->amodId;

	bool reCreation = true;
	if (reCreation) {
		addNewVh2CarPark(vhID,idNode);
		vhOnTheRoad.erase(vh->amodId);
	} else {
		//	//assign trip chain

		DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+ConfigManager::GetInstance().FullConfig().baseGranMS());;
		sim_mob::Trip* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", pnode, "node", pnode, "node");
		sim_mob::SubTrip subTrip("", "Trip", 0, 1, start, DailyTime(), pnode, "node", pnode, "node", "Car");
		tc->addSubTrip(subTrip);
		std::vector<sim_mob::TripChainItem*>  tcs;
		tcs.push_back(tc);
		vh->setTripChain(tcs); //add trip chain


		AMODVirtualCarParkItor it = virtualCarPark.find(idNode);
		if(it!=virtualCarPark.end())
		{
			// access this car park if it already exists
			boost::unordered_map<std::string,Person*> cars = it->second;
			std::cout << "Dest carPark. Before Insertion. Cars Size: " << cars.size() << std::endl;
			cars.insert(std::make_pair(vhID,vh));
			std::cout << "Dest carPark. Inserted. Cars Size: " << cars.size() << std::endl;

			boost::unordered_map<std::string,Person*>::iterator local_it;
			std::cout << "Cars in Car Park : \n";
			for ( local_it = cars.begin(); local_it!= cars.end(); ++local_it ) {
				std::cout << " " << local_it->first << ":" << local_it->second << std::endl;
			}
			std::cout << "-----\n";

			it->second = cars;
		}
		else
		{
			std::cout << "Carpark at the destination node. New car park" << std::endl;
			boost::unordered_map<std::string,Person*> cars = boost::unordered_map<std::string,Person*>();
			cars.insert(std::make_pair(vhID,vh));
			std::cout << "After make_pair" << std::endl;
			virtualCarPark.insert(std::make_pair(idNode,cars));
			std::cout << "Dest carPark. Inserted. Cars Size: " << cars.size() << std::endl;
		}

		//remove vehicle from on the road
		vhInCarPark.insert( std::make_pair(vhID, vh) );
		vhOnTheRoad.erase(vh->amodId);
		nFreeCars++;
		vh->amodVehicle = NULL;
		vh->currStatus = Person::IN_CAR_PARK;
	}
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

	std::map<double, Person::rdSegTravelStats>::const_iterator it =
			ag->getRdSegTravelStatsMap().find(rdSegExitTime);

	ofstream out_TT;
	out_TT.open("out_TT.txt", fstream::out | fstream::app);

	if (it != ag->getRdSegTravelStatsMap().end()){
		double travelTime = (it->first) - (it->second).rdSegEntryTime_;
		std::map<const RoadSegment*, Conflux::rdSegTravelTimes>::iterator itTT = RdSegTravelTimesMap.find((it->second).rdSeg_);
		if (itTT != RdSegTravelTimesMap.end())
		{
			itTT->second.agentCount_ = itTT->second.agentCount_ + 1;
			itTT->second.rdSegTravelTime_ = itTT->second.rdSegTravelTime_ + travelTime;
		}
		else{
			Conflux::rdSegTravelTimes tTimes(travelTime, 1);
			RdSegTravelTimesMap.insert(std::make_pair(ag->getCurrSegment(), tTimes));
		}

		WayPoint w = ag->amodPath.back();
		const RoadSegment *rs = w.roadSegment_;
		std::string segmentID = rs->originalDB_ID.getLogItem();

		std::cout << "Segment ID: "<< segmentID << " ,Segment travel time: " << rdSegExitTime << std::endl;

		if (out_TT.is_open()) {
			out_TT << "Segment ID: " << segmentID << "Segment travel time: " << rdSegExitTime << std::endl;
		}
		else{
			cout << "Unable to open file\n";
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
bool AMODController::insertTravelTime2TmpTable(timeslice frameNumber, std::map<const RoadSegment*, sim_mob::Conflux::rdSegTravelTimes>& rdSegTravelTimesMap)
{
	bool res=false;
	if (ConfigManager::GetInstance().FullConfig().PathSetMode()) {
		//sim_mob::Link_travel_time& data
		std::map<const RoadSegment*, sim_mob::Conflux::rdSegTravelTimes>::const_iterator it = rdSegTravelTimesMap.begin();
		for (; it != rdSegTravelTimesMap.end(); it++){
			Link_travel_time tt;
			DailyTime simStart = ConfigManager::GetInstance().FullConfig().simStartTime();
			std::string aimsun_id = (*it).first->originalDB_ID.getLogItem();
			std::string seg_id = getNumberFromAimsunId(aimsun_id);
			try {
				tt.link_id = boost::lexical_cast<int>(seg_id);
			} catch( boost::bad_lexical_cast const& ) {
				Print() << "Error: seg_id string was not valid" << std::endl;
				tt.link_id = -1;
			}

			tt.start_time = (simStart + sim_mob::DailyTime(frameNumber.ms())).toString();
			double frameLength = ConfigManager::GetInstance().FullConfig().baseGranMS();
			tt.end_time = (simStart + sim_mob::DailyTime(frameNumber.ms() + frameLength)).toString();
			tt.travel_time = (*it).second.rdSegTravelTime_/(*it).second.agentCount_;

			PathSetManager::getInstance()->insertTravelTime2TmpTable(tt);
		}
	}
	return res;
}
void AMODController::testTravelTimePath()
{
	std::string destNodeId="61688";
	std::string carParkId = "75780";
	Node *startNode = nodePool[carParkId];
	Node *endNode = nodePool[destNodeId];

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
			std::cout<<"from node: "<<rs->getStart()->originalDB_ID.getLogItem()<<" to node: "<<rs->getEnd()->originalDB_ID.getLogItem()<<std::endl;
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

			/*
			std::map<const RoadSegment*, Conflux::rdSegTravelTimes>::iterator it = RdSegTravelTimesMap.find(rs);
			if (it==RdSegTravelTimesMap.end()) {
				std::cout << "Warning: no segment found!" << std::endl;
			}
			else {
				//problem at initial stages, road segment times give nonsensical times
				//to check with Simmob team
				//if ((it->second).agentCount_ <= 1e10) {
					//never been traversed (hypothesis) - To check with Simmob team
					//units for below are unknown and needs to be verified.
					//assumptions getLengthOfSegment() returns in cm
					//			  maxSpeed is km/h - assumes car travels at maximum allowed speed if possible
					travelTime += rs->getLengthOfSegment()/(rs->maxSpeed*27.778);
					//std::cout << "Segment Length: " << rs->getLengthOfSegment() << std::endl;
					//std::cout << "Max Speed: " << rs->maxSpeed << std::endl;

				//} else {
				//	travelTime += (it->second).rdSegTravelTime_;
				//}
			}*/
		}
	}
	return travelTime;
}

bool AMODController::findRemainingWayPoints(Person *vh, std::vector < sim_mob::WayPoint > &remainingWPs) {

	//	std::cout << "1: " << vh->amodId << std::endl;

	if (!vh) return false;
	if (vh==NULL) return false;
	if (vh->amodVehicle == 0) {
		return false;
	}
	if (!(vh->amodVehicle)) return false;
	if (vh->currStatus != Person::ON_THE_ROAD) return false;
	//	std::cout << "1: " << vh->amodVehicle->getVehicleID() << std::endl;
	//first, we get the iterators from the amodVehicle Pointer.

	if (vh->amodVehicle != 0) {
		//std::cout << vh->amodId << std::endl;
		std::vector<const sim_mob::RoadSegment*>::iterator pathItr = vh->amodVehicle->getPathIterator();
		std::vector<const sim_mob::RoadSegment*>::iterator pathItrCopy = pathItr;
		std::vector<const sim_mob::RoadSegment*>::iterator pathEnd = vh->amodVehicle->getPathIteratorEnd();

		if (pathItrCopy == pathEnd) {
			return false; //we've arrived at the end.
		}

		//remove the current segment if needed
		//		std::cout << vh->amodId << std::endl;
		//		std::cout << vh->laneID << std::endl;
		//		std::cout << vh->amodVehicle->vehicle_id << std::endl;
		//		std::cout << vh->amodVehicle->getCurrSegment()->getSegmentID() << std::endl;

		if (*pathItrCopy == vh->amodVehicle->getCurrSegment()) {
			pathItrCopy++;
		}

		//loop through the segments remaining and add it to the Waypoint vector
		//std::cout << "Segments remaining:" << std::endl;
		while (pathItrCopy != pathEnd) {
			//std::cout << (*pathItrCopy)->getSegmentID() << std::endl;
			sim_mob::WayPoint wp;
			wp.type_ = WayPoint::ROAD_SEGMENT;
			wp.roadSegment_ = (*pathItrCopy);
			remainingWPs.push_back(wp);
			pathItrCopy++;
		}
		//std::cout << "Segments remaining end" << std::endl;

		return true;
	} else{
		return false;
	}
}

void AMODController::saveVehStat(int currTime)
{
	boost::unordered_map<std::string, Person*>::iterator it = vhOnTheRoad.begin();
	Person* vh;
	Person* vh_cp;
	for(; it != vhOnTheRoad.end(); ++it)
	{
		vh = it->second;
		string amodId;
		amodId = it->first;
		//const RoadSegment* rs = vh->amodVehicle->getCurrSegment();
		if (vh->amodVehicle == NULL) continue;
		vh->amodVehicle->getX();
		vh->amodVehicle->getY();
		vh->amodVehicle->getCurrSegment();


		out_vhsStat << vh->amodId << " " << currTime << " " << vh->amodVehicle->getX() << " " << vh->amodVehicle->getY() << " " << vh->amodVehicle->getCurrSegment() << " onRoad " << " NA" << " NA" << "\n";
	}
	AMODVirtualCarParkItor iter = virtualCarPark.begin();
	for(; iter != virtualCarPark.end(); iter++){
		boost::unordered_map<std::string,Person*> cars = iter->second;
		string nodeId = iter->first;
		cout << "nodeId in vehStat: " << nodeId << endl;
		if (!cars.empty()) {
			boost::unordered_map<std::string,Person*>::iterator local_it;
			//cout << "local_it: first: " << local_it->first;
			for ( local_it = cars.begin(); local_it!= cars.end(); ++local_it ) {
				out_vhsStat << local_it->first << " " << currTime << " " << "NA " << " NA" << " NA " << "NA " << nodeId << " inCarpark" << "\n";
			}
		}
	}
}






//-----------------------------------------------------------------------------
//assignVhs
//-----------------------------------------------------------------------------
void AMODController::assignVhs(std::vector<std::string>& origin, std::vector<std::string>& destination)
{
	/*
	std::cout << "Origin inside assignVhs(): " << std::endl;
	copy(origin.begin(), origin.end(), ostream_iterator<string>(cout, " "));
	std::cout << endl << "---------------" << std::endl;
	std::vector < Person * > justDispatched;
	for (int i =0; i<origin.size() ; i++) //
	{//loop through OD pairs
		std::string originNodeId = origin[i];
		std::cout << "originNodeId: " << originNodeId << std::endl;
		std::string destNodeId = destination[i];
		std::string carParkId;
		std::vector<std::string> carParkIds;

		Node *originNode = nodePool[originNodeId];
		Node *destNode = nodePool[destNodeId];

		//searching for the nearest free vehicle
		std::cout << "Finding nearest vehicle for : " << originNodeId << std::endl;

		std::vector < sim_mob::WayPoint > leastCostPath;
		double bestFreeVehTravelCost;
		Person* vhFree;


		bool freeVehFound = getBestFreeVehicle(originNodeId, &vhFree, carParkId, leastCostPath, bestFreeVehTravelCost);
		if (!freeVehFound)
		{
			std::cout << "No free car found!" << std::endl;
			//throw std::runtime_error("no more vehicles remaining. Throwing exception.");
		} else {
			std::cout << vhFree << std::endl;
			std::cout << "Found: " << vhFree->amodId << " in carpark " << carParkId << std::endl;
		}

		//-------------------------------------------------------------------------
		// Finding the vh which is currently in use with the shortest travel time to origin given the origin
		//First algorithm looks for all vhs which are in service (for(; it != vhOnTheRoad.end(); ++it))
		//for each vh -> retrieves second position from unordered_map and amodId
		//findRemaining waypoints to the current destination
		//calculates travel time for the remaining trip to the current destination

		std::vector<WayPoint> remainingWP;
		std::vector<WayPoint> DestToOriginWP;
		std::vector<WayPoint> CurrentPosToOriginWP;
		std::vector<vector<WayPoint> > vectorWPs;
		string amodIdInService;
		std::vector<string> vhsInUse;
		Person* vhInService;
		vector<Person*> vhsInService;
		std::vector<double> TTVhsInUse;


		boost::unordered_map<std::string, Person*>::iterator it = vhOnTheRoad.begin();

		bool inUseVehFound = false;
		double bestInUseTravelCost = 1e10;
		Person *bestInUseVeh = NULL;
		std::vector < WayPoint > bestVehInUsePath;


		for(; it != vhOnTheRoad.end(); ++it)
		{
			//std::cout << "vhOnTheRoad.begin()->second : " << vhOnTheRoad.begin()->second;
			//std::cout << it->first<<std::endl;

			//vhInService = vhOnTheRoad[it->second];
			amodIdInService = it->first;
			vhInService = it->second;

			std::cout << "amodIdInService: " << amodIdInService << std::endl;
			//find remaining waypoint in current trip
			findRemainingWayPoints(vhInService, remainingWP);

			for (int i=0; i< remainingWP.size();i++) {
				std::cout << remainingWP[i].roadSegment_->getSegmentID() << std::endl;
			}

			//const Node* nextNode;
			//nextNode = vhInService->amodVehicle->getNodeMovingTowards();

			//find the destination node of the current trip
			//Node * endNodeOfCurrentTrip = vhInService->amodVehicle->getPath().end();
			//const sim_mob::RoadSegment* pathEnd = vhInService->amodVehicle->getPath().back();
			const Node* prevDestNode = vhInService->amodPath.back().roadSegment_->getEnd();


			//calculate shortest travel time from current dest to origin

			const sim_mob::RoadSegment* exclude_seg;
			std::vector<const sim_mob::RoadSegment*> blacklist;
			if(exclude_seg)
			{
				blacklist.push_back(exclude_seg);
			}

			//std::vector<WayPoint> DestToOriginWP = stdir->SearchShortestDrivingTimePath(stdir->DrivingTimeVertex( *(pathEnd->getEnd()) ,sim_mob::Default),
			//		stdir->DrivingTimeVertex(*originNode,sim_mob::Default), blacklist, sim_mob::Default);


			//std::vector<WayPoint> remainingWP = stdir->SearchShortestDrivingPath(stdir->DrivingVertex( *nextNode ),
			//		stdir->DrivingVertex(  *(pathEnd->getEnd()) ), blacklist);

			//std::vector<WayPoint> DestToOriginWP = stdir->SearchShortestDrivingPath(stdir->DrivingVertex( *(pathEnd->getEnd())),
			//		stdir->DrivingVertex(*originNode), blacklist);

			std::vector<WayPoint> DestToOriginWP = stdir->SearchShortestDrivingPath(stdir->DrivingVertex( *(prevDestNode)),
					stdir->DrivingVertex(*originNode), blacklist);

			if (DestToOriginWP.size() == 0) {
				std::cout << "OriginNode: " << originNode->nodeId << std::endl;
				std::cout << "No path found!" << std::endl;
			} else {
				inUseVehFound = true;
			}

			mergeWayPoints(remainingWP, DestToOriginWP, CurrentPosToOriginWP);
			std::cout << "Reamining WP" << std::endl;
			for (int i=0; i< remainingWP.size();i++) {
				std::cout << remainingWP[i].roadSegment_->getSegmentID() << std::endl;
			}
			std::cout << "DestToOriginWP WP" << std::endl;
			for (int i=0; i< DestToOriginWP.size();i++) {
				std::cout << DestToOriginWP[i].roadSegment_->getSegmentID() << std::endl;
			}
			std::cout << "CurrentPosToOriginWP WP" << std::endl;
			for (int i=0; i< CurrentPosToOriginWP.size();i++) {
				std::cout << CurrentPosToOriginWP[i].roadSegment_->getSegmentID() << std::endl;
			}


			double timeTaken = calculateTravelTime(CurrentPosToOriginWP);
			if (timeTaken < bestInUseTravelCost) {
				//assign new best in use vehicle
				bestInUseTravelCost = timeTaken;
				bestVehInUsePath = CurrentPosToOriginWP;
				bestInUseVeh = vhInService;
			}

			//			//calculate travel time for each pair of waypoint and collect them in TTVhsInUse vector
			//			TTVhsInUse.push_back(calculateTravelTime(CurrentPosToOriginWP));
			//			//vector of amodId for each travel time from vector TTVhsInUse
			//			vhsInUse.push_back(amodIdInService);
			//			//vector of Person* vhInUse
			//			vhsInService.push_back(vhInService);
			//			//vector of waypoints
			//			vectorWPs.push_back(CurrentPosToOriginWP);

		}

		if (!inUseVehFound && !freeVehFound) {
			std::cout << "ERROR! No path found. Skipping Trip!" << std::endl;
			continue;
		}


		//---------------------------------------------------------------------------------
		//given the stt_freeVh - find which vh can arrive first to the origin (the fastest free or the fastest in use
		//bestFreeVehTravelCost - free vehicles travel cost
		//stt_vhInUse - vehicles in use travel cost
		bool useFreeVeh = false;
		if ( freeVehFound && inUseVehFound) {
			if (bestFreeVehTravelCost <= bestInUseTravelCost ) {
				useFreeVeh = true;
			}
		} else if ( !freeVehFound && inUseVehFound) {
			useFreeVeh = false;
		} else if ( !inUseVehFound) {
			useFreeVeh = true;
		}


		Person *vhAssigned;
		if (useFreeVeh) {
			//assign free veh here
			vhAssigned = vhFree;
			Node *carParkNode = nodePool[carParkId];

			//remove the vehicle from the car park
			if (!removeVhFromCarPark(carParkId, &vhAssigned)) {
				std::cout << "Error! Cannot remove car from car park!" << std::endl;
			}

			// create trip chain
			DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+ConfigManager::GetInstance().FullConfig().baseGranMS());;
			sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", carParkNode, "node", destNode, "node");
			SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), carParkNode, "node", destNode, "node", "Car");
			((Trip*)tc)->addSubTrip(subTrip);

			std::vector<sim_mob::TripChainItem*>  tcs;
			tcs.push_back(tc);
			//vhAssigned->clearTripChain();
			vhAssigned->setTripChain(tcs); //add trip chain

			// set route for vehicle

			const sim_mob::RoadSegment* exclude_seg;
			std::vector<const sim_mob::RoadSegment*> blacklist;
			if(exclude_seg)
			{
				blacklist.push_back(exclude_seg);
			}

			//if provided then remove this part
			std::vector<WayPoint> wp1 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*carParkNode), stdir->DrivingVertex(*originNode),blacklist);
			std::cout<<"starttime: "<<vhAssigned->getStartTime()<<std::endl;

			// find route from origin to destination
			std::vector<WayPoint> wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*originNode), stdir->DrivingVertex(*destNode),blacklist);

			//merge wayPoints
			vector<WayPoint> mergedWP;
			mergeWayPoints(wp1, wp2, mergedWP);

			std::cout << "Merged WayPoionts" << std::endl;
			for (int i=0; i<mergedWP.size(); i++) {
				std::cout << mergedWP[i].roadSegment_->getSegmentID() << std::endl;
			}
			std::cout << std::endl;


			//set path
			vhAssigned->setPath(mergedWP);
			// set event
			eventPub.registerEvent(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH);
			eventPub.subscribe(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH, vhAssigned, &Person::handleAMODEvent, vhAssigned);

			// dispatch vehicle
			dispatchVh(vhAssigned);
			justDispatched.push_back(vhAssigned);

		} else {
			//assign best vehicle in use
			vhAssigned = bestInUseVeh;
			//given origin and destination calculate sttp ->vector<WayPoint> wp2
			const sim_mob::RoadSegment* exclude_seg;
			std::vector<const sim_mob::RoadSegment*> blacklist;
			if(exclude_seg)
			{
				blacklist.push_back(exclude_seg);
			}
			std::vector<WayPoint> wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*originNode),
					stdir->DrivingVertex(*destNode), blacklist);

			//merge wayPoints
			vector<WayPoint> mergedWP;
			mergeWayPoints(bestVehInUsePath, wp2, mergedWP);

			std::cout << "Merged WayPoionts" << std::endl;
			for (int i=0; i<mergedWP.size(); i++) {
				std::cout << mergedWP[i].roadSegment_->getSegmentID() << std::endl;
			}
			std::cout << std::endl;

			//reroute with path
			rerouteWithPath(vhAssigned, mergedWP);
		}
	}

	//add just dispatched vehicles to the road
	for (int i=0; i<justDispatched.size(); i++) {
		vhOnTheRoad.insert(std::make_pair(justDispatched[i]->amodId,justDispatched[i]));

	}
	 */
}


void AMODController::assignVhsFast(std::vector<std::string>& tripID, std::vector<std::string>& origin, std::vector<std::string>& destination, int currTime)
{
	if (out_demandStat.is_open()&& out_vhsStat.is_open()) {
		std::vector < Person * > justDispatched;


		//add origin and destination to complete set
		for (int i =0; i<origin.size() ; i++){
			AmodTrip atrip;
			atrip.tripID = tripID[i];
			atrip.origin = origin[i];
			atrip.destination = destination[i];
			atrip.time = currTime;

			//std::cout << "atrip.tripID = tripID[i]: " << std::endl;


			std::vector<WayPoint> wp1 = getShortestPath(origin[i], destination[i]);
			if (wp1.size() == 0){

				out_demandStat << tripID[i] << " " << origin[i] << " " << destination[i] << " rejected " << "no_path_from_origin_to_destination\n";

				continue; //not possible to service this trip. no possible route.
			}

			serviceBuffer.push_back(atrip);
		}

		std::cout << "Service Buffer Size: " << serviceBuffer.size() << ", Free Cars: " << nFreeCars << ", time: " << currTime << std::endl;

		// work through list using available free cars
		ServiceIterator itr =serviceBuffer.begin();

		while (true) {
			if (nFreeCars <= 0) break; //buffer to prevent getting a car which hasn't been inserted completely
			if (serviceBuffer.size() == 0) break;
			if (itr == serviceBuffer.end()) break;

			std::string originNodeId = itr->origin;
			std::string destNodeId = itr->destination;
			int reqTime = itr->time;
			std::string tripId = itr->tripID;

			Node *originNode = nodePool[originNodeId];
			Node *destNode = nodePool[destNodeId];

			//searching for the nearest free vehicle
			//std::cout << "Finding nearest vehicle for : " << originNodeId << std::endl;

			std::vector < sim_mob::WayPoint > leastCostPath;
			double bestFreeVehTravelCost;
			Person* vhAssigned;
			std::string carParkId;
			bool freeVehFound = getBestFreeVehicle(originNodeId, &vhAssigned, carParkId, leastCostPath, bestFreeVehTravelCost);
			if (!freeVehFound)
			{
				out_demandStat << tripId << " " << originNodeId << " " << destNodeId << " " << reqTime << " rejected " << "no_path_from_carpark_to_origin\n";
				itr = serviceBuffer.erase(itr);
				//itr++;
				continue;
				//throw std::runtime_error("no more vehicles remaining. Throwing exception.");
			}


			Node *carParkNode = nodePool[carParkId];
			// get route for vehicle
			vector<WayPoint> mergedWP;
			if (carParkNode != originNode) {
				std::vector<WayPoint> wp1 = getShortestPath(carParkId, originNodeId);
				std::vector<WayPoint> wp2 = getShortestPath(originNodeId, destNodeId);
				//merge wayPoints
				mergeWayPoints(wp1, wp2, mergedWP);
			} else {
				// find route from origin to destination
				std::vector<WayPoint> wp2 = getShortestPath(originNodeId, destNodeId);
				for (int i=0; i<wp2.size(); i++) {
					if (wp2[i].type_ == WayPoint::ROAD_SEGMENT ) {
						mergedWP.push_back(wp2[i]);
					}
				}
			}

			if (mergedWP.size() <=0) {
				//std::cout << "Merged path is of zero size! Not servicing" << std::endl;
				itr++;
				continue;
			}

			//grab vehicle from car park and send it on its way

			//remove the vehicle from the car park
			if (!removeVhFromCarPark(carParkId, &vhAssigned)) {
				std::cout << "Error! Cannot remove car from car park!" << std::endl;
				itr++;
				continue;
			}

			// create trip chain
			DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+ConfigManager::GetInstance().FullConfig().baseGranMS());;
			sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", carParkNode, "node", destNode, "node");
			SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), carParkNode, "node", destNode, "node", "Car");
			((Trip*)tc)->addSubTrip(subTrip);

			std::vector<sim_mob::TripChainItem*>  tcs;
			tcs.push_back(tc);
			//vhAssigned->clearTripChain();
			vhAssigned->setTripChain(tcs); //add trip chain

			std::cout << "Assigned Path: ";
			for (int i=0; i<mergedWP.size(); i++) {
				std::cout << mergedWP[i].roadSegment_->originalDB_ID.getLogItem() << " -> ";
			}
			std::cout << std::endl;

			vhAssigned->setPath(mergedWP);
			// set event
			eventPub.registerEvent(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH);
			eventPub.subscribe(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH, vhAssigned, &Person::handleAMODEvent, vhAssigned);

			// dispatch vehicle
			dispatchVh(vhAssigned);
			justDispatched.push_back(vhAssigned);


			std::cout << " >>> Servicing " << originNodeId << " -> " << destNodeId << " with: " << vhAssigned->amodId << std::endl;
			AmodTrip at;
			at.origin = originNodeId; //store only the current origin node
			at.destination = destNodeId;
			vhTripMap.insert(std::make_pair(vhAssigned, at));

			double tripDistance;
			double tripDistanceInM;
			WayPoint iterWP;
			const RoadSegment *iterwp;
			std::cout << "Length of segments: ";
			for (int j = 0; j< mergedWP.size(); j++){
				iterWP = mergedWP[j];
				const RoadSegment *rs = iterWP.roadSegment_;
				std::cout << mergedWP[j].roadSegment_->length << " -> ";
				//iterwp = mergedWP[j];
				tripDistance += mergedWP[j].roadSegment_->length;
			}
			tripDistanceInM = tripDistance/100;
			cout << endl;
			cout << "Trip distance: " << tripDistanceInM << endl;

			out_demandStat << tripId << " " << originNodeId << " " << destNodeId << " " << reqTime << " servicing " << "accepted " << " timePickedUp? " << "timeDropped? " << tripDistanceInM << "\n";

			//if (vhOnTheRoad==true){
			//			vhAssigned->amodVehicle->getCurrSegment()
			//		}

			//out_vhsStat << vhAssigned->amodId << " " << currTime << " posX " << "posY " << "inCarpark " << carParkId << " onRoad " << "\n";

			//vhAssigned->amodVehicle->getX();
			//vhAssigned->amodVehicle->getY();
			//pop out the serviced client
			itr = serviceBuffer.erase(itr);
		}


		//add just dispatched vehicles to the road
		for (int i=0; i<justDispatched.size(); i++) {
			vhOnTheRoad.insert(std::make_pair(justDispatched[i]->amodId,justDispatched[i]));
		}

		//find current positions x,y of all vehicles
		// get the positions of the nodes
		//double oX = originNode->location.getX();
		//double oY = originNode->location.getY();

		/*
		 * 	//find the closest car park
	std::vector < std::vector < sim_mob::WayPoint > > carParksToOriginWaypoints;
	for (iter=virtualCarPark.begin(); iter != virtualCarPark.end(); iter++) {
		boost::unordered_map<std::string,Person*> cars = iter->second;
		if (!cars.empty()) {
			//this car park has cars, find distance to node
			double travelCost;

			// add to virtual car park
	AMODVirtualCarParkItor it = virtualCarPark.find(nodeId);
	if(it!=virtualCarPark.end())
		 */

	}
	else cout << "Unable to open out_vhsStat or out_demandStat";

}



void AMODController::frame_output(timeslice now)
{

}

} /* namespace AMOD */
} /* namespace sim_mob */
