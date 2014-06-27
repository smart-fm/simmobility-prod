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
	return true;
}

Entity::UpdateStatus AMODController::frame_tick(timeslice now)
{
	//std::cout<< "Time:" << now.ms() << std::endl;

	if(test==0)
	{
		//testSecondVh();
		//inserting vehicles to virtual carParks at the nodes
		populateCarParks(20);
		//myFile.open("/home/km/Dropbox/research/autonomous/automated-MoD/simMobility_implementation/txtFiles/About10.txt");
		myFile.open("/home/haroldsoh/Development/simmobility/dataFiles/About10.txt");
		lastReadLine = "";
		test=1;
	}

	if(test==1)
	{
		//reading the file of time, destination and origin
		int current_time = now.ms();
		std::cout << "current_time in frame_tick: " << current_time << std::endl;
		std::vector<string> origin;
		std::vector<string> destination;
		std::cout << "Reading the demand file... " << std::endl;
		readDemandFile(current_time, origin, destination);

		assignVhs(origin, destination);

		//cout << "Origin after reading the file: "<<endl;
		//copy(origin.begin(), origin.end(), ostream_iterator<string>(cout, " "));

		//testVh();
		//test=2;
	}

#if 0
	if(now.frame()>150 & now.frame()<300)
	{
		testTravelTimePath();
	}

	if(test==1)
	{
		Person *vh = vhOnTheRoad.begin()->second;
		if(vh->getRole())
		{
			//			Driver *driver = (Driver*)vh->getRole();

			//			std::string segid = vh->getRole()->getVehicle()->getCurrSegment()->originalDB_ID.getLogItem();

			if(vh->getCurrSegment())
			{
				std::string segid = vh->getCurrSegment()->originalDB_ID.getLogItem();

				if(segid.find("3440") != std::string::npos)
				{
					std::vector<std::string> segs;
					segs.push_back("34396");
					segs.push_back("34354");

					std::vector<sim_mob::WayPoint> path;

					for(int i=0;i<segs.size();++i)
					{
						RoadSegment *seg = segPool[segs[i]];
						WayPoint wp(seg);
						path.push_back(wp);
					}
					rerouteWithPath(vh,path);

					test=2;
				}
			}
		}

		//		AMODObj obj;
		//		AMODObjContainer obj1(obj);
		//		char c[20]="\0";
		//		sprintf(c,"xxx+%d",now.frame());
		//		obj1.data = std::string(c);

		//		sim_mob:
		//		eventPub.publish(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_, vh, AMODRerouteEventArgs(obj1));

	}
#endif

	// return continue, make sure agent not remove from main loop
	return Entity::UpdateStatus::Continue;
}

void AMODController::populateCarParks(int numberOfVhsAtNode = 10)
{
	//carpark population
	std::vector<std::string> carParkIds;
	carParkIds.push_back("61688");
	carParkIds.push_back("66508");
	carParkIds.push_back("83620");
	carParkIds.push_back("45666");
	carParkIds.push_back("65298");

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

void AMODController::readDemandFile(int current_time, vector<string>& origin, vector<string>& destination)
{
	string line;
	string time_;
	string origin_;
	string destination_;
	//ifstream myfile; //("/home/km/Dropbox/research/autonomous/automated-MoD/simMobility_implementation/txtFiles/About10.txt");
	std::cout << "cureent_time inside the readDemandFile: " << current_time << std::endl;

	if (myFile.is_open())
	{
		std::cout << "file is open. I am inside the readDemandFile function." << std::endl;

		while(myFile.good())
		{
			if(lastReadLine =="")
			{
				std::getline (myFile,line);
			}
			else{
				line = lastReadLine;
				lastReadLine = "";
			}
			std::stringstream ss;
			ss << line;
			ss >> time_ >> origin_ >> destination_;
			std::cout << "myFile >> time_:" << time_ << " >> origin_: " << origin_ << " >> destination_: " << destination_ << std::endl;
			int time = atoi(time_.c_str());
			std::cout << "Time Converted to int: " << time << std::endl;
			std::cout << "(time*100) : " << time*100 << std::endl;
			std::cout << "current_time: " << current_time << std::endl;
			if ((time*100) <= current_time)
			{
				origin.push_back(origin_);
				destination.push_back(destination_);
				std::cout << "origin.push_back(origin_) " << std::endl;
				copy(origin.begin(), origin.end(), ostream_iterator<string>(cout, " "));
				lastReadLine = "";
			} else {
				lastReadLine = line;
				break;
			}

		}
		//myFile.close();
	}
	else cout << "Unable to open file";

	//origin.pop_back();
	//destination.pop_back();

	//cout << "OD Pairs"<< endl;
	//std::cout << myFile << std::endl;
	std::cout << "Origin: " << std::endl;
	copy(origin.begin(), origin.end(), ostream_iterator<string>(cout, " "));
	std::cout << endl << "---------------" << std::endl;
	std::cout << "Destination: " << std::endl;
	copy(destination.begin(), destination.end(), ostream_iterator<string>(cout, " "));
	std::cout << endl << "---------------" << std::endl;
	std::cout << "Time: " << time_ << std::endl;
	std::cout << endl << "---------------" << std::endl;
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

	std::cout<<ConfigManager::GetInstance().FullConfig().simStartTime().getValue()<<std::endl;
	sim_mob::Person* person = new sim_mob::Person("AMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
	std::cout<<"starttime: "<<person->getStartTime()<<std::endl;
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
	person->currStatus = Person::IN_CAR_PARK;
}


//void AMODController::findAllFreeVhs()
//{
//	//TODO
//}
//

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
			travelCost = getTravelTimePath(iter->first , originId, wps);

			//if the travel cost is less than the best (or no car found yet), assign new best
			if ((travelCost < bestTravelCost) || (!freeCarFound)) {
				bestTravelCost = travelCost;
				bestCarParkIter = iter;
				leastCostPath = wps;
				freeCarFound = true;
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
		cars.erase((*vh)->amodId);
		//it->second.erase((*vh)->amodId);
		it->second = cars;
		vhInCarPark.erase( (*vh)->amodId );
		(*vh)->currStatus = Person::ON_THE_ROAD;
		return true;
	}

	return false;
}


void AMODController::mergeWayPoints(const std::vector<sim_mob::WayPoint>& carparkToOrigin, const std::vector<sim_mob::WayPoint> &originToDestination, std::vector<sim_mob::WayPoint>& mergedWP)
{
	//mergedWP.push_back(carparkToOrigin);
	//mergedWP.push_back(originToDestination);

	for (int i=0; i<carparkToOrigin.size(); i++) {
		if (carparkToOrigin[i].roadSegment_->getSegmentID() != 0) {
			mergedWP.push_back(carparkToOrigin[i]);
		}
	}

	for (int i=0; i<originToDestination.size(); i++) {
		if (originToDestination[i].roadSegment_->getSegmentID() != 0) {
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
void AMODController::handleVHArrive(Person* vh)
{
	std::cout << "handleVHArrive function " << std::endl;

	WayPoint w = vh->amodPath.back();
	const RoadSegment *rs = w.roadSegment_;
	const Node *enode = rs->getEnd();

	std::string idNode = enode->originalDB_ID.getLogItem();// "aimsun-id":"123456"
	std::cout << "NodeId before: " << idNode << std::endl;

	char chars[] = "aimsun-id:,\"";
	for (unsigned int i = 0; i < strlen(chars); ++i)
	{
		idNode.erase (std::remove(idNode.begin(), idNode.end(), chars[i]), idNode.end());
	}
	std::cout << "NodeId after: " << idNode << std::endl;

	std::string vhID = vh->amodId;
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
	vhOnTheRoad.erase(vh->amodId);
	vhInCarPark.insert( std::make_pair(vhID, vh) );
	vh->currStatus = Person::IN_CAR_PARK;
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
	out_TT.open("/home/km/workspace/simmobility2/dev/Basic/out_TT.txt", fstream::out | fstream::app);

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
	out_TT.close();
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
			std::map<const RoadSegment*, Conflux::rdSegTravelTimes>::iterator it = RdSegTravelTimesMap.find(rs);
			if (it!=RdSegTravelTimesMap.end()) {
				std::cout << "Warning: no segment found!" << std::endl;
			}
			else {
				//problem at initial stages, road segment times give nonsensical times
				//to check with Simmob team
				if ((it->second).agentCount_ <= 1) {
					//never been traversed (hypothesis) - To check with Simmob team
					//units for below are unknown and needs to be verified.
					//assumptions getLengthOfSegment() returns in cm
					//			  maxSpeed is km/h - assumes car travels at maximum allowed speed if possible
					travelTime += rs->getLengthOfSegment()/(rs->maxSpeed*27.778);
					std::cout << "Segment Length: " << rs->getLengthOfSegment() << std::endl;
					std::cout << "Max Speed: " << rs->maxSpeed << std::endl;

				} else {
					travelTime += (it->second).rdSegTravelTime_;
				}
			}
		}
	}
	return travelTime;
}

void AMODController::findRemainingWayPoints(Person *vh, std::vector < sim_mob::WayPoint > &remainingWPs) {

	//first, we get the iterators from the amodVehicle Pointer.
	std::vector<const sim_mob::RoadSegment*>::iterator pathItr = vh->amodVehicle->getPathIterator();
	std::vector<const sim_mob::RoadSegment*>::iterator pathItrCopy = pathItr;
	std::vector<const sim_mob::RoadSegment*>::iterator pathEnd = vh->amodVehicle->getPathIteratorEnd();

	//loop through the segments remaining and add it to the Waypoint vector
	std::cout << "Segments remaining:" << std::endl;
	while (pathItrCopy != pathEnd) {
		std::cout << (*pathItrCopy)->getSegmentID() << std::endl;
		sim_mob::WayPoint wp;
		wp.type_ = WayPoint::ROAD_SEGMENT;
		wp.roadSegment_ = (*pathItrCopy);
		remainingWPs.push_back(wp);
		pathItrCopy++;
	}
	std::cout << "Segments remaining end" << std::endl;

	return;
}


void AMODController::testVh()
{
	//carpark population
	std::vector<string> carParkIds;
	carParkIds.push_back("61688");
	carParkIds.push_back("66508");
	carParkIds.push_back("83620");
	carParkIds.push_back("45666");
	carParkIds.push_back("65298");

	std::cout << "CarparkIds: " << std::endl;
	copy(carParkIds.begin(), carParkIds.end(), ostream_iterator<string>(cout, " "));
	std::cout << endl << "---------------" << std::endl;

	int vhsInTheSystem = 20;

	int k = 0;
	std::vector<Person*> vhs;
	//while (!carParkIds.empty())
	for(int j = 0; j<carParkIds.size(); j++)
	{
		for(int i = 0; i<vhsInTheSystem; ++i)
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
			addNewVh2CarPark(vhId,carParkId);
		}
		//carParkIds.pop_back();
		std::cout << "Cars inserted. Left carparkIds: " << std::endl;
		copy(carParkIds.begin(), carParkIds.end(), ostream_iterator<string>(cout, " "));
		std::cout << endl << "---------------" << std::endl;
	}

	//reading the file
	string timeD;
	int current_time;
	std::vector<string> origin;
	std::vector<string> destination;
	ifstream myFile ("/home/km/Dropbox/research/autonomous/automated-MoD/simMobility_implementation/txtFiles/About10.txt");
	std::cout << "Reading the demand file... " << std::endl;

	readDemandFile(current_time, origin, destination);

	cout << "Origin after reading the file: "<<endl;
	copy(origin.begin(), origin.end(), ostream_iterator<string>(cout, " "));

	for (int i =0; i<origin.size() ; i++) //
	{
		std::string originNodeId = origin[i];
		std::string destNodeId = destination[i];
		std::string carParkId;
		Person* vh;
		std::cout << "Finding nearest vehicle for : " << originNodeId << std::endl;
		if (!findNearestFreeVehicle(originNodeId, nodePool, carParkId, &vh))
		{
			std::cout << "No car found!" << std::endl;
			throw std::runtime_error("no more vehicles remaining. Throwing exception.");
			break;
		}
		std::cout << vh << std::endl;
		std::cout << "Found: " << vh->amodId << " in carpark " << carParkId << std::endl;

		//Get the cark park and origin and destination nodes
		Node *carParkNode = nodePool[carParkId];
		Node *originNode = nodePool[originNodeId];
		Node *destNode = nodePool[destNodeId];

		// create trip chain
		DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+ConfigManager::GetInstance().FullConfig().baseGranMS());;
		sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", carParkNode, "node", destNode, "node");
		SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), carParkNode, "node", destNode, "node", "Car");
		((Trip*)tc)->addSubTrip(subTrip);

		std::vector<sim_mob::TripChainItem*>  tcs;
		tcs.push_back(tc);

		vh->setTripChain(tcs); //add trip chain

		// set route for vehicle

		const sim_mob::RoadSegment* exclude_seg;
		std::vector<const sim_mob::RoadSegment*> blacklist;
		if(exclude_seg)
		{
			blacklist.push_back(exclude_seg);
		}
		std::vector<WayPoint> wp1 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*carParkNode), stdir->DrivingVertex(*originNode),blacklist);
		std::cout<<"starttime: "<<vh->getStartTime()<<std::endl;

		// find route from origin to destination
		std::vector<WayPoint> wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*originNode), stdir->DrivingVertex(*destNode),blacklist);

		//Merge two sets of way points
		std::vector<WayPoint> mergedWP;
		mergeWayPoints(wp1, wp2, mergedWP);

		std::cout << "Traveltime wp1: " << calculateTravelTime(wp1) << std::endl;
		std::cout << "Traveltime wp2: " << calculateTravelTime(wp2) << std::endl;
		std::cout << "Traveltime mwp: " << calculateTravelTime(mergedWP) << std::endl;

		std::cout << "Waypoint 1 :\n";
		for (int j=0; j<wp1.size(); j++) {
			std::cout << wp1[j].roadSegment_->getSegmentID() << std::endl;
		}
		std::cout << "End Waypint 1\n";

		std::cout << "Waypoint 2 :\n";
		for (int j=0; j<wp2.size(); j++) {
			std::cout << wp2[j].roadSegment_->getSegmentID() << std::endl;
		}
		std::cout << "End Waypint 2\n";

		//set path
		//std::vector<WayPoint> path;
		vh->setPath(mergedWP);
		std::cout << "Waypoint list :\n";
		for (int j=0; j<mergedWP.size(); j++) {
			std::cout << mergedWP[j].roadSegment_->getSegmentID() << std::endl;
		}
		std::cout << "End Waypint list\n";
		// set event
		eventPub.registerEvent(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH);
		eventPub.subscribe(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH, vh, &Person::handleAMODEvent, vh);

		// dispatch vehicle
		dispatchVh(vh);
		vhOnTheRoad.insert(std::make_pair(vh->amodId,vh));

	}
}


void AMODController::assignVhs(std::vector<std::string>& origin, std::vector<std::string>& destination)
{
	for (int i =0; i<origin.size() ; i++) //
	{
		std::string originNodeId = origin[i];
		std::string destNodeId = destination[i];
		std::string carParkId;
		Person* vh;

		//findAllFreeVhs();

		std::cout << "Finding nearest vehicle for : " << originNodeId << std::endl;

		std::vector < sim_mob::WayPoint > leastCostPath;
		double bestTravelCost;

		if (!getBestFreeVehicle(originNodeId, &vh, carParkId, leastCostPath, bestTravelCost))
		{
			std::cout << "No free car found!" << std::endl;
			//throw std::runtime_error("no more vehicles remaining. Throwing exception.");
			break;
		} else {
			//remove the vehicle from the car park and dispatch
			if (!removeVhFromCarPark(carParkId, &vh)) {
				std::cout << "Error! Cannot remove car from car park!" << std::endl;
			}
		}
		std::cout << vh << std::endl;
		std::cout << "Found: " << vh->amodId << " in carpark " << carParkId << std::endl;

		//print out least cost path


		//Get the cark park and origin and destination nodes
		Node *carParkNode = nodePool[carParkId];
		Node *originNode = nodePool[originNodeId];
		Node *destNode = nodePool[destNodeId];

		// create trip chain
		DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+ConfigManager::GetInstance().FullConfig().baseGranMS());;
		sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", carParkNode, "node", destNode, "node");
		SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), carParkNode, "node", destNode, "node", "Car");
		((Trip*)tc)->addSubTrip(subTrip);

		std::vector<sim_mob::TripChainItem*>  tcs;
		tcs.push_back(tc);

		vh->setTripChain(tcs); //add trip chain

		// set route for vehicle

		const sim_mob::RoadSegment* exclude_seg;
		std::vector<const sim_mob::RoadSegment*> blacklist;
		if(exclude_seg)
		{
			blacklist.push_back(exclude_seg);
		}
		std::vector<WayPoint> wp1 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*carParkNode), stdir->DrivingVertex(*originNode),blacklist);
		std::cout<<"starttime: "<<vh->getStartTime()<<std::endl;

		// find route from origin to destination
		std::vector<WayPoint> wp2 = stdir->SearchShortestDrivingPath(stdir->DrivingVertex(*originNode), stdir->DrivingVertex(*destNode),blacklist);

		//Merge two sets of way points
		std::vector<WayPoint> mergedWP;
		mergeWayPoints(wp1, wp2, mergedWP);

		std::cout << "Traveltime wp1: " << calculateTravelTime(wp1) << std::endl;
		std::cout << "Traveltime wp2: " << calculateTravelTime(wp2) << std::endl;
		std::cout << "Traveltime mwp: " << calculateTravelTime(mergedWP) << std::endl;

		std::cout << "Waypoint 1 :\n";
		for (int j=0; j<wp1.size(); j++) {
			std::cout << wp1[j].roadSegment_->getSegmentID() << std::endl;
		}
		std::cout << "End Waypint 1\n";

		std::cout << "Waypoint 2 :\n";
		for (int j=0; j<wp2.size(); j++) {
			std::cout << wp2[j].roadSegment_->getSegmentID() << std::endl;
		}
		std::cout << "End Waypint 2\n";

		//set path
		//std::vector<WayPoint> path;
		vh->setPath(mergedWP);
		std::cout << "Waypoint list :\n";
		for (int j=0; j<mergedWP.size(); j++) {
			std::cout << mergedWP[j].roadSegment_->getSegmentID() << std::endl;
		}
		std::cout << "End Waypint list\n";
		// set event
		eventPub.registerEvent(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH);
		eventPub.subscribe(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH, vh, &Person::handleAMODEvent, vh);

		// dispatch vehicle
		dispatchVh(vh);
		vhOnTheRoad.insert(std::make_pair(vh->amodId,vh));

	}
}

void AMODController::frame_output(timeslice now)
{

}

} /* namespace AMOD */
} /* namespace sim_mob */
