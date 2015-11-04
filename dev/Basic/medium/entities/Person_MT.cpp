//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Person_MT.hpp"

#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "config/MT_Config.hpp"
#include "entities/conflux/SegmentStats.hpp"
#include "entities/misc/PublicTransit.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/roles/RoleFactory.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/network/WayPoint.hpp"
#include "path/PT_RouteChoiceLuaProvider.hpp"
#include "util/DailyTime.hpp"

using namespace std;
using namespace sim_mob;
using namespace sim_mob::medium;

Person_MT::Person_MT(const std::string& src, const MutexStrategy& mtxStrat, int id, std::string databaseID)
: Person(src, mtxStrat, id, databaseID),
isQueuing(false), distanceToEndOfSegment(0.0), drivingTimeToEndOfLink(0.0), remainingTimeThisTick(0.0),
requestedNextSegStats(NULL), canMoveToNextSegment(NONE), nextLinkRequired(NULL), currSegStats(NULL), currLane(NULL),
prevRole(NULL), currRole(NULL), nextRole(NULL)		
{
}

Person_MT::Person_MT(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc)
: Person(src, mtxStrat, tc),
isQueuing(false), distanceToEndOfSegment(0.0), drivingTimeToEndOfLink(0.0), remainingTimeThisTick(0.0),
requestedNextSegStats(NULL), canMoveToNextSegment(NONE), nextLinkRequired(NULL), currSegStats(NULL), currLane(NULL),
prevRole(NULL), currRole(NULL), nextRole(NULL)
{
	convertPublicTransitODsToTrips();
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

void Person_MT::convertPublicTransitODsToTrips()
{
	if (!ConfigManager::GetInstance().FullConfig().isPublicTransitEnabled())
	{
		return;
	}

	std::vector<TripChainItem*>::iterator tripChainItem;
	bool brokenBusTravel = false;
	std::vector<TripChainItem*>::iterator brokenBusTravelItem;
	for (tripChainItem = tripChain.begin(); tripChainItem != tripChain.end(); ++tripChainItem)
	{
		if (brokenBusTravel)
		{
			break;
		}
		if ((*tripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			Trip* trip = dynamic_cast<Trip*>(*tripChainItem);
			std::string originId = boost::lexical_cast<std::string>(trip->origin.node->getNodeId());
			std::string destId = boost::lexical_cast<std::string>(trip->destination.node->getNodeId());
			(*tripChainItem)->startLocationId = originId;
			(*tripChainItem)->endLocationId = destId;
			std::vector<sim_mob::SubTrip>& subTrips = (dynamic_cast<sim_mob::Trip*>(*tripChainItem))->getSubTripsRW();
			std::vector<SubTrip>::iterator itSubTrip = subTrips.begin();
			std::vector<sim_mob::SubTrip> newSubTrips;
			while (itSubTrip != subTrips.end())
			{
				if (itSubTrip->origin.type == WayPoint::NODE && itSubTrip->destination.type == WayPoint::NODE)
				{
					if (itSubTrip->mode == "BusTravel" || itSubTrip->mode == "MRT")
					{
						std::vector<sim_mob::OD_Trip> odTrips;
						bool ret = sim_mob::PT_RouteChoiceLuaProvider::getPTRC_Model().getBestPT_Path(itSubTrip->origin.node->getNodeId(),
								itSubTrip->destination.node->getNodeId(), odTrips);
						if (ret)
						{
							ret = makeODsToTrips(&(*itSubTrip), newSubTrips, odTrips);
						}
						if (!ret)
						{
							tripChain.clear();
							return;
						}
					}
					else if (itSubTrip->mode == "Walk")
					{
						std::string originId = boost::lexical_cast<std::string>(itSubTrip->origin.node->getNodeId());
						std::string destId = boost::lexical_cast<std::string>(itSubTrip->destination.node->getNodeId());

						itSubTrip->startLocationId = originId;
						itSubTrip->endLocationId = destId;
						itSubTrip->startLocationType = "NODE";
						itSubTrip->endLocationType = "NODE";
					}
					else if (itSubTrip->mode.find("Car Sharing") != std::string::npos)
					{
						std::string originId = boost::lexical_cast<std::string>(itSubTrip->origin.node->getNodeId());
						std::string destId = boost::lexical_cast<std::string>(itSubTrip->destination.node->getNodeId());

						itSubTrip->startLocationId = originId;
						itSubTrip->endLocationId = destId;
						itSubTrip->startLocationType = "NODE";
						itSubTrip->endLocationType = "NODE";
						itSubTrip->mode = "Sharing";

						const StreetDirectory& streetDirectory = StreetDirectory::Instance();
						std::vector<WayPoint> wayPoints = streetDirectory.SearchShortestDrivingPath(*itSubTrip->origin.node, *itSubTrip->destination.node);
						double travelTime = 0.0;
						for (std::vector<WayPoint>::iterator it = wayPoints.begin(); it != wayPoints.end(); it++)
						{
							if (it->type == WayPoint::ROAD_SEGMENT)
							{
								travelTime += it->roadSegment->getDefaultTravelTime();
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

	if (brokenBusTravel)
	{
		tripChainItem = brokenBusTravelItem;
		while (tripChainItem != tripChain.end())
		{
			delete *tripChainItem;
			tripChainItem = tripChain.erase(tripChainItem);
		}
	}
}

void Person_MT::insertWaitingActivityToTrip()
{
	std::vector<TripChainItem*>::iterator tripChainItem;
	for (tripChainItem = tripChain.begin(); tripChainItem != tripChain.end();
			++tripChainItem)
	{
		if ((*tripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
		{
			std::vector<SubTrip>::iterator itSubTrip[2];
			std::vector<sim_mob::SubTrip>& subTrips =
					(dynamic_cast<sim_mob::Trip*> (*tripChainItem))->getSubTripsRW();

			itSubTrip[1] = subTrips.begin();
			itSubTrip[0] = subTrips.begin();
			while (itSubTrip[1] != subTrips.end())
			{
				if (itSubTrip[1]->mode == "BusTravel"
						&& itSubTrip[0]->mode != "WaitingBusActivity")
				{
					if (itSubTrip[1]->origin.type == WayPoint::BUS_STOP)
					{
						sim_mob::SubTrip subTrip;
						subTrip.itemType = TripChainItem::getItemType(
																	"WaitingBusActivity");
						subTrip.origin = itSubTrip[1]->origin;
						subTrip.originType = itSubTrip[1]->originType;
						subTrip.destination = itSubTrip[1]->destination;
						subTrip.destinationType = itSubTrip[1]->destinationType;
						subTrip.startLocationId = itSubTrip[1]->origin.busStop->getStopCode();
						subTrip.endLocationId = itSubTrip[1]->destination.busStop->getStopCode();
						subTrip.startLocationType = "BUS_STOP";
						subTrip.endLocationType = "BUS_STOP";
						subTrip.mode = "WaitingBusActivity";
						subTrip.ptLineId = itSubTrip[1]->ptLineId;
						itSubTrip[1] = subTrips.insert(itSubTrip[1], subTrip);
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
	if (src == "XML_TripChain" || src == "DAS_TripChain" || src == "AMOD_TripChain" || src == "BusController")
	{
		setStartTime((*currTripChainItem)->startTime.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
	}
	else
	{
		setStartTime((*currTripChainItem)->startTime.getValue());
	}
	
	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		currSubTrip = ((dynamic_cast<sim_mob::Trip*> (*currTripChainItem))->getSubTripsRW()).begin();
		
		// if the first trip chain item is passenger, create waitBusActivityRole
		if (currSubTrip->mode == "BusTravel")
		{
			const RoleFactory<Person_MT> *rf = RoleFactory<Person_MT>::getInstance();
			currRole = rf->createRole("waitBusActivity", this);
			nextRole = rf->createRole("passenger", this);
		}
		
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
		const SubTrip* subTrip = NULL;

		if (tci->itemType == TripChainItem::IT_TRIP)
		{
			subTrip = &(*currSubTrip);
		}
		nextRole = rf->createRole(tci, subTrip, this);
	}

	changeRole();
	return true;
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

	//Trip is about the change, collect the Metrics	
	//if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	//{
	//	aggregateSubTripMetrics();
	//}
	//
	//serializeTripChainItem(currTripChainItem); 

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

Entity::UpdateStatus Person_MT::checkTripChain()
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

	//Null out our trip chain, remove the "removed" flag, and return
	clearToBeRemoved();
	return UpdateStatus(UpdateStatus::RS_CONTINUE, prevParams, currParams);
}
