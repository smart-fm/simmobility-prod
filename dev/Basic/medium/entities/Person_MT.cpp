//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Person_MT.hpp"

using namespace sim_mob;

Person_MT::Person_MT(const std::string& src, const MutexStrategy& mtxStrat, int id = -1, std::string databaseID = "")
: Person(src, mtxStrat, id, databaseID),
isQueuing(false), distanceToEndOfSegment(0.0), drivingTimeToEndOfLink(0.0), remainingTimeThisTick(0.0),
requestedNextSegStats(nullptr), canMoveToNextSegment(NONE), nextLinkRequired(nullptr), currSegStats(nullptr), currLane(NULL)
{
}

Person_MT::Person_MT(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc)
: Person(src, mtxStrat, tc),
isQueuing(false), distanceToEndOfSegment(0.0), drivingTimeToEndOfLink(0.0), remainingTimeThisTick(0.0),
requestedNextSegStats(nullptr), canMoveToNextSegment(NONE), nextLinkRequired(nullptr), currSegStats(nullptr), currLane(NULL)
{
	convertODsToTrips();
	insertWaitingActivityToTrip();
	assignSubtripIds();
}

Person_MT::~Person_MT()
{
}

void Person_MT::convertODsToTrips()
{
	ConfigParams& config = ConfigManager::GetInstanceRW().FullConfig();
	if (!config.publicTransitEnabled)
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
			Trip* trip = dynamic_cast<Trip*> (*tripChainItem);
			std::vector<sim_mob::OD_Trip> odTrips;
			std::string originId = boost::lexical_cast<std::string>(trip->origin.node_->getID());
			std::string destId = boost::lexical_cast<std::string>(trip->destination.node_->getID());
			(*tripChainItem)->startLocationId = originId;
			(*tripChainItem)->endLocationId = destId;
			std::vector<sim_mob::SubTrip>& subTrips = (dynamic_cast<sim_mob::Trip*> (*tripChainItem))->getSubTripsRW();
			std::vector<SubTrip>::iterator itSubTrip = subTrips.begin();
			std::vector<sim_mob::SubTrip> newSubTrips;
			while (itSubTrip != subTrips.end())
			{
				if (itSubTrip->origin.type_ == WayPoint::NODE
						&& itSubTrip->destination.type_ == WayPoint::NODE)
				{
					if (itSubTrip->mode == "BusTravel" || itSubTrip->mode == "MRT")
					{
						std::vector<sim_mob::OD_Trip> odTrips;
						std::string originId = boost::lexical_cast<std::string>(itSubTrip->origin.node_->getID());
						std::string destId = boost::lexical_cast<std::string>(itSubTrip->destination.node_->getID());

						bool ret = sim_mob::PT_RouteChoiceLuaProvider::getPTRC_Model().getBestPT_Path(originId, destId, odTrips);
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
						std::string originId = boost::lexical_cast<std::string>(itSubTrip->origin.node_->getID());
						std::string destId = boost::lexical_cast<std::string>(itSubTrip->destination.node_->getID());

						itSubTrip->startLocationId = originId;
						itSubTrip->endLocationId = destId;
						itSubTrip->startLocationType = "NODE";
						itSubTrip->endLocationType = "NODE";
					}
					else if (itSubTrip->mode.find("Car Sharing") != std::string::npos)
					{
						std::string originId = boost::lexical_cast<std::string>(itSubTrip->origin.node_->getID());
						std::string destId = boost::lexical_cast<std::string>(itSubTrip->destination.node_->getID());

						itSubTrip->startLocationId = originId;
						itSubTrip->endLocationId = destId;
						itSubTrip->startLocationType = "NODE";
						itSubTrip->endLocationType = "NODE";
						itSubTrip->mode = "Sharing";

						const StreetDirectory& streetDirectory = StreetDirectory::instance();
						StreetDirectory::VertexDesc source, destination;
						source = streetDirectory.DrivingVertex(*itSubTrip->origin.node_);
						destination = streetDirectory.DrivingVertex(*itSubTrip->destination.node_);
						std::vector<WayPoint> wayPoints = streetDirectory.SearchShortestDrivingPath(source, destination);
						double travelTime = 0.0;
						for (std::vector<WayPoint>::iterator it = wayPoints.begin(); it != wayPoints.end(); it++)
						{
							if (it->type_ == WayPoint::ROAD_SEGMENT)
							{
								travelTime += it->roadSegment_->getDefaultTravelTime();
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
					if (itSubTrip[1]->origin.type_ == WayPoint::BUS_STOP)
					{
						sim_mob::SubTrip subTrip;
						subTrip.itemType = TripChainItem::getItemType(
																	"WaitingBusActivity");
						subTrip.origin = itSubTrip[1]->origin;
						subTrip.originType = itSubTrip[1]->originType;
						subTrip.destination = itSubTrip[1]->destination;
						subTrip.destinationType = itSubTrip[1]->destinationType;
						subTrip.startLocationId = itSubTrip[1]->origin.busStop_->getBusstopno_();
						subTrip.endLocationId = itSubTrip[1]->destination.busStop_->getBusstopno_();
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
			const RoleFactory& rf = ConfigManager::GetInstance().FullConfig().getRoleFactory();
			currRole = rf.createRole("waitBusActivity", this);
			nextRole = rf.createRole("passenger", this);
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