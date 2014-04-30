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
#include "entities/Person.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/RoadSegment.hpp"
#include "geospatial/UniNode.hpp"
#include "entities/misc/TripChain.hpp"
#include "workers/Worker.hpp"
#include "metrics/Frame.hpp"
#include <utility>
#include <stdexcept>

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
	//TODO
	if(test==0)
	{
		testOneVh();
		test=1;
	}

	if(now.frame()>150 & now.frame()<300)
	{
		testTravelTimePath();
	}
#if 0
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
	sim_mob::Person* person = new sim_mob::Person("FMOD_TripChain", ConfigManager::GetInstance().FullConfig().mutexStategy(), tcs);
	std::cout<<"starttime: "<<person->getStartTime()<<std::endl;
	person->parentEntity = this;
	person->amodId = id;

	// add to virtual car park
	AMODVirtualCarParkItor it = virtualCarPark.find(nodeId);
	if(it!=virtualCarPark.end())
	{
		// access this car park before
		boost::unordered_map<std::string,Person*> cars = it->second;
		cars.insert(std::make_pair(id,person));
		virtualCarPark.insert(std::make_pair(nodeId,cars));
	}
	else
	{
		boost::unordered_map<std::string,Person*> cars = boost::unordered_map<std::string,Person*>();
		cars.insert(std::make_pair(id,person));
		virtualCarPark.insert(std::make_pair(nodeId,cars));
	}
}
bool AMODController::getVhFromCarPark(std::string& carParkId,Person** vh)
{
	AMODVirtualCarParkItor it = virtualCarPark.find(carParkId);
	if(it==virtualCarPark.end()){ throw std::runtime_error("no this car park..."); }

	boost::unordered_map<std::string,Person*> cars = it->second;
	if(!cars.empty())
	{
		boost::unordered_map<std::string,Person*>::iterator firstCarIt = cars.begin();
		*vh = firstCarIt->second;
		cars.erase(firstCarIt);

		return true;
	}

	return false;
}
bool AMODController::dispatchVh(Person* vh)
{
	this->currWorkerProvider->scheduleForBred(vh);
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
	}
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
void AMODController::testOneVh()
{
	std::string carParkId = "75780";
	std::string vhId = "amod-1";
	addNewVh2CarPark(vhId,carParkId);

	Person* vh = NULL;
	if(!getVhFromCarPark(carParkId,&vh))
	{ throw std::runtime_error("no vh"); return; }

	// modify trip
	std::string destNodeId="61688";
	Node *startNode = nodePool[carParkId];
	Node *endNode = nodePool[destNodeId];
	DailyTime start(ConfigManager::GetInstance().FullConfig().simStartTime().getValue()+ConfigManager::GetInstance().FullConfig().baseGranMS());;
	sim_mob::TripChainItem* tc = new sim_mob::Trip("-1", "Trip", 0, -1, start, DailyTime(), "", startNode, "node", endNode, "node");
	SubTrip subTrip("-1", "Trip", 0, -1, start, DailyTime(), startNode, "node", endNode, "node", "Car");
	((Trip*)tc)->addSubTrip(subTrip);

	std::vector<sim_mob::TripChainItem*>  tcs;
	tcs.push_back(tc);

	vh->setTripChain(tcs);
	std::cout<<"starttime: "<<vh->getStartTime()<<std::endl;

	// make dummy path 9286 9264
//	RoadSegment *seg1 = segPool["9286"];
//	WayPoint wp1(seg1);
//	RoadSegment *seg2 = segPool["9264"];
//	WayPoint wp2(seg2);

	std::vector<std::string> segs;
	segs.push_back("9282");
	segs.push_back("34500");
	segs.push_back("34514");
	segs.push_back("34488");
	segs.push_back("34400");
	segs.push_back("34398");
	segs.push_back("34378");

	std::vector<WayPoint> path;

	for(int i=0;i<segs.size();++i)
	{
		RoadSegment *seg = segPool[segs[i]];
		WayPoint wp(seg);
		path.push_back(wp);
	}

#if 1
	vh->setPath(path);
#endif


//	unsigned int curTickMS = (frameTicks)*ConfigManager::GetInstance().FullConfig().baseGranMS();
//	vh->setStartTime(curTickMS);

	// event related
	eventPub.registerEvent(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH);
	eventPub.subscribe(sim_mob::event::EVT_AMOD_REROUTING_REQUEST_WITH_PATH, vh, &Person::handleAMODEvent, vh);

	dispatchVh(vh);

	vhOnTheRoad.insert(std::make_pair(vh->amodId,vh));


}
void AMODController::frame_output(timeslice now)
{

}




} /* namespace AMOD */
} /* namespace sim_mob */
