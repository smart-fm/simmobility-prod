//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Person_MT.hpp"

#include <boost/lexical_cast.hpp>
#include <map>
#include <numeric>
#include <stdexcept>
#include "behavioral/lua/WithindayLuaModel.hpp"
#include "behavioral/WithindayHelper.hpp"
#include "behavioral/params/WithindayModeParams.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/MT_Config.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "entities/TravelTimeManager.hpp"
#include "entities/TrainController.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/network/WayPoint.hpp"
#include "path/PT_RouteChoiceLuaProvider.hpp"
#include "entities/incident/IncidentManager.hpp"
#include "util/DailyTime.hpp"
#include "geospatial/streetdir/RailTransit.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "entities/PT_Statistics.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;

namespace
{
std::map<int, std::string> setModeMap()
{
	// 1 for public bus; 2 for MRT/LRT; 3 for private bus; 4 for drive1;
	// 5 for shared2; 6 for shared3+; 7 for motor; 8 for walk; 9 for taxi
	//mode_idx_ref = { 1 : 3, 2 : 5, 3 : 3, 4 : 1, 5 : 6, 6 : 6, 7 : 8, 8 : 2, 9 : 4 }
	std::map<int, std::string> res;
	res[-1] = "Invalid";
	res[1] = "BusTravel";
	res[2] = "MRT";
	res[3] = "PrivateBus";
	res[4] = "Car";
	res[5] = "Car Sharing 2";
	res[6] = "Car Sharing 3";
	res[7] = "Motorcycle";
	res[8] = "Walk";
	res[9] = "Taxi";
	return res;
}

const std::map<int,std::string> modeMap = setModeMap();

const std::string& getModeString(int idx)
{
	if((idx < 1 || idx > 9) && (idx != -1))
	{
		throw std::runtime_error("Invalid mode index");
	}
	return modeMap.find(idx)->second;
}
}

Person_MT::Person_MT(const std::string& src, const MutexStrategy& mtxStrat, int id, std::string databaseID)
: Person(src, mtxStrat, id, databaseID),
isQueuing(false), distanceToEndOfSegment(0.0), drivingTimeToEndOfLink(0.0), remainingTimeThisTick(0.0),
requestedNextSegStats(nullptr), canMoveToNextSegment(NONE), currSegStats(nullptr), currLane(nullptr),
prevRole(nullptr), currRole(nullptr), nextRole(nullptr), numTicksStuck(0)
{
}

Person_MT::Person_MT(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc)
: Person(src, mtxStrat, tc),
isQueuing(false), distanceToEndOfSegment(0.0), drivingTimeToEndOfLink(0.0), remainingTimeThisTick(0.0),
requestedNextSegStats(nullptr), canMoveToNextSegment(NONE), currSegStats(nullptr), currLane(nullptr),
prevRole(nullptr), currRole(nullptr), nextRole(nullptr), numTicksStuck(0)
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	std::string ptPathsetStoredProcName = cfg.getDatabaseProcMappings().procedureMappings["pt_pathset"];
	try{
		convertPublicTransitODsToTrips(PT_NetworkCreater::getInstance(), ptPathsetStoredProcName);
	} catch(PT_PathsetLoadException& exception){
		Print()<<"[PT pathset]load pt pathset failed!"<<"["<<exception.originNode<<","<<exception.destNode<<"]"<<std::endl;
	}
	insertWaitingActivityToTrip();
	assignSubtripIds();
	if (!tripChain.empty())
	{
		initTripChain();
	}
}

Person_MT::~Person_MT()
{
	safe_delete_item(prevRole);
	safe_delete_item(currRole);
	safe_delete_item(nextRole);
}

void Person_MT::FindMrtTripsAndPerformRailTransitRoute(std::vector<sim_mob::OD_Trip>& matchedTrips)
{
  typename std::vector<sim_mob::OD_Trip>::iterator itr=matchedTrips.begin();
  std::vector<sim_mob::OD_Trip> newODTrips;
  while(itr!=matchedTrips.end())
  {
       if((*itr).tType==TRAIN_EDGE)
       {
           std::string src=(*itr).startStop;
           std::string end=(*itr).endStop;
           size_t pos = 0;
           if((pos=(src.find("/"))) != std::string::npos)
		   {
        	   src = src.substr(0, pos);
		   }

           if((pos=(end.find("/"))) != std::string::npos)
           {
               end = end.substr(0, pos);
           }

           std::vector<std::string> railPath=RailTransit::getInstance().fetchBoardAlightStopSeq(src,end);
           if(railPath.empty())
        	   return;
           std::vector<sim_mob::OD_Trip> odTrips=splitMrtTrips(railPath);
           typename std::vector<sim_mob::OD_Trip>::iterator itrMrtODTrips=odTrips.begin();
           while(itrMrtODTrips!=odTrips.end())
           {
        	   newODTrips.push_back(*itrMrtODTrips);
        	   itrMrtODTrips++;
           }

       }
       else
       {
          newODTrips.push_back(*itr);
       }
         itr++;
  }
  matchedTrips=newODTrips;

}


 sim_mob::OD_Trip Person_MT::CreateMRTSubTrips(std::string src,std::string dest)
{

	 sim_mob::OD_Trip odTrip;
	 sim_mob::TrainStop* destStop = sim_mob::PT_NetworkCreater::getInstance().findMRT_Stop(dest);
	WayPoint wayPointDestStop;//=WayPoint(destStop);
	WayPoint wayPointSrcStop;
     if (destStop)
	 {
    	 wayPointDestStop = WayPoint(destStop);
	 }

 	sim_mob::TrainStop* srcStop = sim_mob::PT_NetworkCreater::getInstance().findMRT_Stop(src);
 	//WayPoint wayPointDestStop;//=WayPoint(destStop);
      if (srcStop)
 	 {
     	 wayPointSrcStop = WayPoint(srcStop);
 	 }
	sim_mob::SubTrip subTrip;
	odTrip.startStop=srcStop->getStopName();;
	odTrip.endStop=destStop->getStopName();
	odTrip.sType=2;
	odTrip.eType=2;
	odTrip.tType=TRAIN_EDGE;
	std::map<std::string ,std::map<std::string ,std::vector<PT_NetworkEdge>>> mrtEdgeMap=PT_NetworkCreater::getInstance().MRTStopdgesMap;
	std::vector<PT_NetworkEdge> edgeVector=mrtEdgeMap[odTrip.startStop][odTrip.endStop];
	typename std::vector<PT_NetworkEdge>::iterator iter=edgeVector.begin();
	std::string selServiceLine="";
	if(iter!=edgeVector.end())
	{
		PT_NetworkEdge selEdge=*iter;

		if(edgeVector.size()>1)
		{
			while(iter!=edgeVector.end())
			{
				std::string serviceLine=iter->getServiceLine();
				if(boost::iequals(src.substr(0,2),serviceLine.substr(0,2)))
				{
					selServiceLine=serviceLine;
					selEdge=*iter;
				}
				iter++;
			}
		}
		odTrip.id=selEdge.getEdgeId();
		odTrip.serviceLine=selEdge.getServiceLine();
		odTrip.serviceLines=selEdge.getServiceLines();

	}

	return odTrip;

}

 std::vector<sim_mob::OD_Trip>  Person_MT::splitMrtTrips(std::vector<std::string> railPath)
{
	vector<std::string>::iterator it=railPath.begin();
	//vector<std::string>::iterator itrEnd=railPath.end();
	//vector<std::string>::iterator prev=railPath.begin();
	std::string first=(*it);
    std::string second=*(it+1);
    sim_mob::TrainStop* firststop=sim_mob::PT_NetworkCreater::getInstance().MRTStopsMap[first];
    sim_mob::TrainStop* nextstop=sim_mob::PT_NetworkCreater::getInstance().MRTStopsMap[second];
    if((boost::iequals(firststop->getStopName(),nextstop->getStopName())))
    {
    	railPath.erase(it);
    	it=railPath.begin();
    }
    vector<std::string>::iterator itrEnd=railPath.end();
    std::string last=*(itrEnd-1);
    std::string seclast=*(itrEnd-2);
    firststop=sim_mob::PT_NetworkCreater::getInstance().MRTStopsMap[seclast];
    nextstop=sim_mob::PT_NetworkCreater::getInstance().MRTStopsMap[last];
    if((boost::iequals(firststop->getStopName(),nextstop->getStopName())))
    {
    	railPath.erase(itrEnd-1);

    }

	std::vector<sim_mob::OD_Trip> odTrips;
	std::string src=(*it);
	std::string end="";
	std::string prev="";
	while(it!=railPath.end())
	{
       if((*it).find("NE")!= std::string::npos)
       {
          if(!boost::iequals(prev, ""))
          {
    	    end=prev;
          //make subtrip of start and end
    	    odTrips.push_back(CreateMRTSubTrips(src,end));
          }
          src=(*it);
          end=*(++it);
          //make NE subtrip;
          odTrips.push_back(CreateMRTSubTrips(src,end));
          ++it;
          if(it==railPath.end())
        	  break;
          src=(*it);
          prev="";
          continue;
       }

       if(it+1==railPath.end())
       {
    	   end=(*it);
    	   //make subtrip
    	   odTrips.push_back(CreateMRTSubTrips(src,end));
       }

       prev=(*it);
       it++;

	}
	return odTrips;

}


void Person_MT::changeToNewTrip(const std::string& stationName)
{
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	std::string ptPathsetStoredProcName = cfg.getDatabaseProcMappings().procedureMappings["pt_pathset_dis"];
	TripChainItem* trip = (*currTripChainItem);
	sim_mob::TrainStop* stop = sim_mob::PT_NetworkCreater::getInstance2().findMRT_Stop(stationName);
	if(stop){
		const Node* node = stop->getRandomStationSegment()->getParentLink()->getFromNode();
		trip->origin = WayPoint(node);
		std::vector<sim_mob::SubTrip>& subTrips = (dynamic_cast<sim_mob::Trip*>(trip))->getSubTripsRW();
		sim_mob::SubTrip newSubTrip;
		newSubTrip.origin = trip->origin;
		newSubTrip.destination = trip->destination;
		newSubTrip.originType = trip->originType;
		newSubTrip.destinationType = trip->destinationType;
		newSubTrip.travelMode = "Car";
		subTrips.clear();
		bool isLoaded=false;
		subTrips.push_back(newSubTrip);
		if(newSubTrip.travelMode=="BusTravel"){
			try{
				convertPublicTransitODsToTrips(PT_NetworkCreater::getInstance2(), ptPathsetStoredProcName);
				isLoaded = true;
			} catch(PT_PathsetLoadException& exception){
				Print()<<"[PT pathset]load pt pathset failed!"<<"["<<exception.originNode<<","<<exception.destNode<<"]"<<std::endl;
				isLoaded = false;
			}
			insertWaitingActivityToTrip();
			assignSubtripIds();
		}
		PT_RerouteInfo rerouteInfo;
		rerouteInfo.personId = this->getDatabaseId();
		rerouteInfo.stopNo = stop->getStopName();
		rerouteInfo.lastRoleType = this->getRole()->roleType;
		rerouteInfo.travelMode = newSubTrip.travelMode;
		rerouteInfo.startNodeId = newSubTrip.origin.node->getNodeId();
		rerouteInfo.destNodeId = newSubTrip.destination.node->getNodeId();
		rerouteInfo.isPT_loaded = isLoaded;
		messaging::MessageBus::PostMessage(PT_Statistics::getInstance(),
				STORE_PERSON_REROUTE, messaging::MessageBus::MessagePtr(new PT_RerouteInfoMessage(rerouteInfo)), true);
		currSubTrip = subTrips.begin();
		isFirstTick = true;
	}
}

void Person_MT::convertPublicTransitODsToTrips(PT_Network& ptNetwork,const std::string&  ptPathsetStoredProcName)
{
	std::vector<TripChainItem*>::iterator tripChainItemIt;
	for (tripChainItemIt = tripChain.begin(); tripChainItemIt != tripChain.end(); ++tripChainItemIt)
	{
		if ((*tripChainItemIt)->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			unsigned int start_time = ((*tripChainItemIt)->startTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime())/1000); // start time in seconds
			TripChainItem* trip = (*tripChainItemIt);
			std::string originId = boost::lexical_cast<std::string>(trip->origin.node->getNodeId());
			std::string destId = boost::lexical_cast<std::string>(trip->destination.node->getNodeId());
			trip->startLocationId = originId;
			trip->endLocationId = destId;
			std::vector<sim_mob::SubTrip>& subTrips = (dynamic_cast<sim_mob::Trip*>(*tripChainItemIt))->getSubTripsRW();
			std::vector<SubTrip>::iterator itSubTrip = subTrips.begin();
			std::vector<sim_mob::SubTrip> newSubTrips;
			while (itSubTrip != subTrips.end())
			{
				if (itSubTrip->origin.type == WayPoint::NODE && itSubTrip->destination.type == WayPoint::NODE)
				{
					if (itSubTrip->getMode() == "BusTravel" || itSubTrip->getMode() == "MRT")
					{
						std::vector<sim_mob::OD_Trip> odTrips;

						std::string dbid = this->getDatabaseId();
						bool ret = sim_mob::PT_RouteChoiceLuaProvider::getPTRC_Model().getBestPT_Path(itSubTrip->origin.node->getNodeId(),
																		itSubTrip->destination.node->getNodeId(),itSubTrip->startTime.getValue(), odTrips, dbid, start_time,ptPathsetStoredProcName);

						FindMrtTripsAndPerformRailTransitRoute(odTrips);

						if (ret)
						{
							ret = makeODsToTrips(&(*itSubTrip), newSubTrips, odTrips, ptNetwork);
						}
						if (!ret)
						{
							tripChain.clear();
							return;
						}
					}
					else if (itSubTrip->getMode() == "Walk")
					{
						std::string originId = boost::lexical_cast<std::string>(itSubTrip->origin.node->getNodeId());
						std::string destId = boost::lexical_cast<std::string>(itSubTrip->destination.node->getNodeId());

						itSubTrip->startLocationId = originId;
						itSubTrip->endLocationId = destId;
						itSubTrip->startLocationType = "NODE";
						itSubTrip->endLocationType = "NODE";
					}
					else if (itSubTrip->getMode().find("Car Sharing") != std::string::npos || itSubTrip->getMode() == "PrivateBus")
					{
						std::string originId = boost::lexical_cast<std::string>(itSubTrip->origin.node->getNodeId());
						std::string destId = boost::lexical_cast<std::string>(itSubTrip->destination.node->getNodeId());

						itSubTrip->startLocationId = originId;
						itSubTrip->endLocationId = destId;
						itSubTrip->startLocationType = "NODE";
						itSubTrip->endLocationType = "NODE";
						if(itSubTrip->getMode() != "PrivateBus")
						{
							itSubTrip->travelMode = "Sharing"; // modify mode name for RoleFactory
						}
						
						const StreetDirectory& streetDirectory = StreetDirectory::Instance();
						std::vector<WayPoint> wayPoints = streetDirectory.SearchShortestDrivingPath<Node, Node>(*itSubTrip->origin.node, *itSubTrip->destination.node);
						double travelTime = 0.0;
						const TravelTimeManager* ttMgr = TravelTimeManager::getInstance();
						for (std::vector<WayPoint>::iterator it = wayPoints.begin(); it != wayPoints.end(); it++)
						{
							if (it->type == WayPoint::LINK)
							{
								travelTime += ttMgr->getDefaultLinkTT(it->link);
							}
						}
						itSubTrip->endTime = DailyTime(travelTime * 1000);
					}
				}
				++itSubTrip;
			}

			if (!newSubTrips.empty())
			{
				subTrips.clear();
				subTrips = newSubTrips;
			}
		}
	}
}

void Person_MT::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
	switch(eventId)
	{
	case EVT_DISRUPTION_CHANGEROUTE:
	{
		const ChangeRouteEventArgs& exArgs = MSG_CAST(ChangeRouteEventArgs, args);
		const std::string stationName = exArgs.getStationName();
		changeToNewTrip(stationName);
		break;
	}
	}

	if(currRole){
		currRole->onParentEvent(eventId, ctxId, sender, args);
	}
}
void Person_MT::insertWaitingActivityToTrip()
{
	std::vector<TripChainItem*>::iterator tripChainItem;
	for (tripChainItem = tripChain.begin(); tripChainItem != tripChain.end(); ++tripChainItem)
	{
		if ((*tripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			std::vector<SubTrip>::iterator itSubTrip[2];
			std::vector<sim_mob::SubTrip>& subTrips = (dynamic_cast<sim_mob::Trip*> (*tripChainItem))->getSubTripsRW();

			itSubTrip[1] = subTrips.begin();
			itSubTrip[0] = subTrips.begin();
			while (itSubTrip[1] != subTrips.end())
			{
				if (itSubTrip[1]->getMode() == "BusTravel" && itSubTrip[0]->getMode() != "WaitingBusActivity")
				{
					if (itSubTrip[1]->origin.type == WayPoint::BUS_STOP)
					{
						sim_mob::SubTrip subTrip;
						subTrip.itemType = TripChainItem::getItemType("WaitingBusActivity");
						subTrip.origin = itSubTrip[1]->origin;
						subTrip.originType = itSubTrip[1]->originType;
						subTrip.destination = itSubTrip[1]->destination;
						subTrip.destinationType = itSubTrip[1]->destinationType;
						subTrip.startLocationId = itSubTrip[1]->origin.busStop->getStopCode();
						subTrip.endLocationId = itSubTrip[1]->destination.busStop->getStopCode();
						subTrip.startLocationType = "BS";
						subTrip.endLocationType = "BS";
						subTrip.travelMode = "WaitingBusActivity";
						subTrip.ptLineId = itSubTrip[1]->ptLineId;
						subTrip.edgeId = itSubTrip[1]->edgeId;
						itSubTrip[1] = subTrips.insert(itSubTrip[1], subTrip);
					}
				}
				else if(itSubTrip[1]->getMode() == "MRT" && itSubTrip[0]->getMode() != "WaitingTrainActivity")
				{
					if (itSubTrip[1]->origin.type == WayPoint::TRAIN_STOP)
					{
						sim_mob::SubTrip subTrip;
						subTrip.itemType = TripChainItem::getItemType("WaitingTrainActivity");
						const std::string& firstStationName = itSubTrip[1]->origin.trainStop->getStopName();
						std::string lineId = itSubTrip[1]->serviceLine;
						Platform* platform =TrainController<Person_MT>::getInstance()->getPlatform(lineId, firstStationName);
						if (platform && itSubTrip[1]->destination.type == WayPoint::TRAIN_STOP)
						{
							subTrip.origin = WayPoint(platform);
							subTrip.originType = itSubTrip[1]->originType;
							subTrip.startLocationId = platform->getPlatformNo();
							const std::string& secondStationName = itSubTrip[1]->destination.trainStop->getStopName();
							platform = TrainController<Person_MT>::getInstance()->getPlatform(lineId, secondStationName);
							if (platform)
							{
								subTrip.destination = WayPoint(platform);
								subTrip.destinationType = itSubTrip[1]->destinationType;
								subTrip.endLocationId = platform->getPlatformNo();
								subTrip.startLocationType = "PT";
								subTrip.endLocationType = "PT";
								subTrip.travelMode = "WaitingTrainActivity";
								subTrip.serviceLine = itSubTrip[1]->serviceLine;
								std::string serviceLine=itSubTrip[1]->serviceLine;
								subTrip.ptLineId = itSubTrip[1]->ptLineId;
								subTrip.edgeId = itSubTrip[1]->edgeId;
								itSubTrip[1]->origin = subTrip.origin;
								itSubTrip[1]->destination = subTrip.destination;
								itSubTrip[1]->startLocationId = subTrip.startLocationId;
								itSubTrip[1]->startLocationType = "PT";
								itSubTrip[1]->endLocationId = subTrip.endLocationId;
								itSubTrip[1]->endLocationType = "PT";
								itSubTrip[1]->serviceLine = subTrip.serviceLine;
								itSubTrip[1] = subTrips.insert(itSubTrip[1],subTrip);
							}
							else
							{
								Print() << "[PT pathset] train trip failed:[" << firstStationName << "]|[" << secondStationName << "]--["<< lineId<<"] - Invalid start/end stop for PT edge" << std::endl;
							}
						}
					}
				}
				itSubTrip[0] = itSubTrip[1];
				itSubTrip[1]++;
			}
		}
	}
}

void Person_MT::assignSubtripIds()
{
	for (std::vector<TripChainItem*>::iterator tcIt = tripChain.begin(); tcIt != tripChain.end(); tcIt++)
	{
		if ((*tcIt)->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			sim_mob::Trip* trip = dynamic_cast<sim_mob::Trip*> (*tcIt);
			std::string tripId = trip->tripID;
			std::stringstream stIdstream;
			std::vector<sim_mob::SubTrip>& subTrips = trip->getSubTripsRW();
			int stNo = 0;
			for (std::vector<sim_mob::SubTrip>::iterator stIt = subTrips.begin(); stIt != subTrips.end(); stIt++)
			{
				stNo++;
				stIdstream << tripId << "_" << stNo;
				(*stIt).tripID = stIdstream.str();
				stIdstream.str(std::string());
			}
		}
	}
}

void Person_MT::initTripChain()
{
	currTripChainItem = tripChain.begin();
	const std::string& src = getAgentSrc();
	DailyTime startTime = (*currTripChainItem)->startTime;
	if (src == "XML_TripChain" || src == "DAS_TripChain" || src == "AMOD_TripChain" || src == "BusController")
	{
		startTime = DailyTime((*currTripChainItem)->startTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
		setStartTime((*currTripChainItem)->startTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
	}
	else
	{
		setStartTime((*currTripChainItem)->startTime.getValue());
	}
	
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		currSubTrip = ((dynamic_cast<sim_mob::Trip*> (*currTripChainItem))->getSubTripsRW()).begin();
		currSubTrip->startTime = startTime;

		if (!updateOD(*currTripChainItem))
		{ 
			//Offer some protection
			throw std::runtime_error("Trip/Activity mismatch, or unknown TripChainItem subclass.");
		}
	}

	setNextPathPlanned(false);
	isFirstTick = true;
}

bool Person_MT::updatePersonRole()
{	
	if (!nextRole)
	{
		const RoleFactory<Person_MT> *rf = RoleFactory<Person_MT>::getInstance();
		const TripChainItem *tci = *(this->currTripChainItem);
		const SubTrip* subTrip = nullptr;

		if (tci->itemType == TripChainItem::IT_TRIP)
		{
			subTrip = &(*currSubTrip);
		}
		nextRole = rf->createRole(tci, subTrip, this);
	}

	changeRole();
	return true;
}

std::string Person_MT::chooseModeEnRoute(const Trip& trip, unsigned int originNode, const DailyTime& curTime) const
{
	WithindayModelsHelper wdHelper;
	ConfigParams& cfg = ConfigManager::GetInstanceRW().FullConfig();
	std::string ptPathsetStoredProcName = cfg.getDatabaseProcMappings().procedureMappings["pt_pathset_dis"];
	WithindayModeParams modeParams = wdHelper.buildModeChoiceParams(trip, originNode, curTime,ptPathsetStoredProcName);
	modeParams.unsetDrive1Availability();
	modeParams.unsetMotorAvailability();
	modeParams.unsetShare2Availability();
	modeParams.unsetShare3Availability();
	modeParams.unsetWalkAvailability();
	modeParams.unsetPrivateBusAvailability();
	modeParams.unsetMrtAvailability();
	int chosenMode = WithindayLuaProvider::getWithindayModel().chooseMode(personInfo, modeParams);
	return getModeString(chosenMode);
}

void Person_MT::setStartTime(unsigned int value)
{
	sim_mob::Entity::setStartTime(value);
	if (currRole)
	{
		currRole->setArrivalTime(value + ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
	}
}

vector<BufferedBase *> Person_MT::buildSubscriptionList()
{
	//First, add the x and y co-ordinates
	vector<BufferedBase *> subsList;
	subsList.push_back(&xPos);
	subsList.push_back(&yPos);

	//Now, add our own properties.
	if (this->getRole())
	{
		vector<BufferedBase*> roleParams = this->getRole()->getSubscriptionParams();

		//Append the subsList with all elements in roleParams
		subsList.insert(subsList.end(), roleParams.begin(), roleParams.end());
	}

	return subsList;
}

void Person_MT::changeRole()
{
	safe_delete_item(prevRole);
	prevRole = currRole;
	currRole = nextRole;
	nextRole = nullptr;
}

bool Person_MT::advanceCurrentTripChainItem()
{
	if (currTripChainItem == tripChain.end()) /*just a harmless basic check*/
	{
		return false;
	}

	// current role (activity or sub-trip level role)[for now: only subtrip] is about to change, time to collect its movement metrics(even activity performer)
	if (currRole != nullptr)
	{
		TravelMetric currRoleMetrics = currRole->Movement()->finalizeTravelTimeMetric();
		currRole->Movement()->resetTravelTimeMetric(); //sorry for manual reset, just a precaution for now
		serializeSubTripChainItemTravelTimeMetrics(currRoleMetrics, currTripChainItem, currSubTrip);
	}

	//first check if you just need to advance the subtrip
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		//don't advance to next tripchainItem immediately, check the subtrip first
		bool res = advanceCurrentSubTrip();

		//subtrip advanced successfully, no need to advance currTripChainItem
		if (res)
		{
			return res;
		}
	}

	//do the increment
	++currTripChainItem;

	if (currTripChainItem == tripChain.end())
	{
		//but tripchain items are also over, get out !
		return false;
	}

	//so far, advancing the tripchainitem has been successful
	//Also set the currSubTrip to the beginning of trip , just in case
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		currSubTrip = resetCurrSubTrip();
	}

	return true;
}

Entity::UpdateStatus Person_MT::checkTripChain(unsigned int currentTime)
{
	if (tripChain.empty())
	{
		return UpdateStatus::Done;
	}

	//advance the trip, sub-trip or activity....
	if (!isFirstTick)
	{
		if (!(advanceCurrentTripChainItem()))
		{
			return UpdateStatus::Done;
		}
		if(isTripValid())
		{
			currSubTrip->startTime = DailyTime(currentTime);
		}
	}
	
	//must be set to false whenever trip chain item changes. And it has to happen before a probable creation of (or changing to) a new role
	setNextPathPlanned(false);

	//Create a new Role based on the trip chain type
	updatePersonRole();

	//Update our origin/destination pair.
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{ 
		//put if to avoid & evade bus trips, can be removed when everything is ok
		updateOD(*currTripChainItem, &(*currSubTrip));
	}

	//currentTipchainItem or current sub-trip are changed
	//so OD will be changed too,
	//therefore we need to call frame_init regardless of change in the role
	unsetInitialized();

	//Create a return type based on the differences in these Roles
	vector<BufferedBase*> prevParams;
	vector<BufferedBase*> currParams;
	
	if (prevRole)
	{
		prevParams = prevRole->getSubscriptionParams();
	}
	
	if (currRole)
	{
		currParams = currRole->getSubscriptionParams();
	}
	
	if (isFirstTick && currRole)
	{
		currRole->setArrivalTime(startTime + ConfigManager::GetInstance().FullConfig().simStartTime().getValue());
	}
	
	isFirstTick = false;

	//remove the "removed" flag, and return
	clearToBeRemoved();
	return UpdateStatus(UpdateStatus::RS_CONTINUE, prevParams, currParams);
}

void Person_MT::log(std::string line) const
{
	Log() << line;
}
