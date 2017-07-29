//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Person.hpp"

#include <algorithm>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

//For debugging
#include "roles/Role.hpp"
#include "entities/misc/TripChain.hpp"
#include "entities/roles/activityRole/ActivityPerformer.hpp"
#include "util/GeomHelpers.hpp"
#include "util/DebugFlags.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "logging/Log.hpp"
#include "geospatial/network/Node.hpp"
#include "geospatial/network/PT_Stop.hpp"
#include "entities/misc/TripChain.hpp"
#include "workers/Worker.hpp"
#include "message/MessageBus.hpp"
#include "path/PT_RouteChoiceLuaModel.hpp"
#include "path/PT_RouteChoiceLuaProvider.hpp"
#include "entities/params/PT_NetworkEntities.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "geospatial/streetdir/RailTransit.hpp"
#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "entities/misc/TripChain.hpp"
#endif
//
using std::map;
using std::string;
using std::vector;
using namespace sim_mob;
using namespace sim_mob::event;
typedef Entity::UpdateStatus UpdateStatus;


namespace
{
// default lowest age
const int DEFAULT_LOWEST_AGE = 20;
// default highest age
const int DEFAULT_HIGHEST_AGE = 60;
} //End unnamed namespace
sim_mob::BasicLogger& CarTravelTimeLogger  = sim_mob::Logger::log("Car_Trave_time.csv");
sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, int id, std::string databaseID)
: Agent(mtxStrat, id), personDbId(databaseID), agentSrc(src), age(0), resetParamsRequired(false), isFirstTick(true), useInSimulationTravelTime(false),
nextPathPlanned(false), originNode(), destNode(), currLinkTravelStats(nullptr)
{
}

sim_mob::Person::Person(const std::string& src, const MutexStrategy& mtxStrat, const std::vector<sim_mob::TripChainItem*>& tc)
: Agent(mtxStrat), personDbId(tc.front()->getPersonID()), agentSrc(src), tripChain(tc), age(0), resetParamsRequired(false),
isFirstTick(true), useInSimulationTravelTime(false), nextPathPlanned(false), originNode(), destNode(), currLinkTravelStats(nullptr)
{
}

sim_mob::Person::~Person()
{
}

void sim_mob::Person::setTripChain(const vector<TripChainItem *>& tripChain)
{
	//delete the previous trip chain
	clear_delete_vector(this->tripChain);

	this->tripChain = tripChain;

	//Initialise the trip chain - as the references to current trip chain and the
	//current sub trip need to be updated
	initTripChain();
}

void sim_mob::Person::load(const map<string, string>& configProps)
{
}

void Person::rerouteWithBlacklist(const std::vector<const sim_mob::Link*>& blacklisted)
{
}

void sim_mob::Person::onEvent(event::EventId eventId, sim_mob::event::Context ctxId, event::EventPublisher* sender, const event::EventArgs& args)
{
}

UpdateStatus sim_mob::Person::frame_init(timeslice now)
{
	return Entity::UpdateStatus::Done;
}

Entity::UpdateStatus sim_mob::Person::frame_tick(timeslice now)
{
	return UpdateStatus::Done;
}

void sim_mob::Person::frame_output(timeslice now)
{
}

bool sim_mob::Person::updateOD(sim_mob::TripChainItem * tc, const sim_mob::SubTrip *subtrip)
{
	return tc->setPersonOD(this, subtrip);
}

std::vector<sim_mob::SubTrip>::iterator sim_mob::Person::resetCurrSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*currTripChainItem);

	if (!trip)
	{
		throw std::runtime_error("non sim_mob::Trip cannot have subtrips");
	}

	return trip->getSubTripsRW().begin();
}


bool sim_mob::Person::makeODsToTrips(SubTrip* curSubTrip, std::vector<sim_mob::SubTrip>& newSubTrips, const std::vector<sim_mob::OD_Trip>& matchedTrips,  PT_Network& ptNetwork)
{
	bool ret = true;
	bool invalidFlag = false;

	if (!matchedTrips.empty())
	{
		std::vector<sim_mob::OD_Trip>::const_iterator it = matchedTrips.begin();
		while (it != matchedTrips.end())
		{
			sim_mob::SubTrip subTrip;
			WayPoint source;
			WayPoint dest;
			std::string sSrc = (*it).startStop;
			std::string sEnd = (*it).endStop;
			std::string srcType;
			std::string endType;
			int sType = (*it).sType;
			int eType = (*it).eType;

			if (it->tType == sim_mob::BUS_EDGE && (sType != 1 || eType != 1))
			{
				invalidFlag = true;
			}

			if (it->tType == sim_mob::TRAIN_EDGE && (sType != 2 || eType != 2))
			{
				invalidFlag = true;
			}

			if(invalidFlag)
			{
				std::stringstream msg;
				msg << __func__ << ": Invalid PT path - Invalid start/end stop for PT edge for Person "
				    << personDbId << ", travelling from " << originNode.node->getNodeId() << " to "
				    << destNode.node->getNodeId() << " at time "
				    << (*currSubTrip).startTime.getStrRepr() << "\n";
				Warn() << msg.str();
				ret = false;
				break;
			}

			switch (eType)
			{
			case 0:
			{
				endType = "N";
				int id = boost::lexical_cast<unsigned int>(sEnd);
				const RoadNetwork* rn = RoadNetwork::getInstance();
				const sim_mob::Node* node = rn->getById(rn->getMapOfIdvsNodes() , id);
				if (node)
				{
					dest = WayPoint(node);
				}
				break;
			}
			case 1:
			{
				endType = "BS";
				sim_mob::BusStop* stop = sim_mob::BusStop::findBusStop(sEnd);
				if (stop)
				{
					dest = WayPoint(stop);
				}
				break;
			}
			case 2:
			{
				endType = "MS";
				sim_mob::TrainStop* stop = ptNetwork.findMRT_Stop(sEnd);
				if (stop)
				{
					dest = WayPoint(stop);
				}
				break;
			}
			}

			switch (sType)
			{
			case 0:
			{
				srcType = "N";
				int id = boost::lexical_cast<unsigned int>(sSrc);
				const RoadNetwork* rn = RoadNetwork::getInstance();
				const sim_mob::Node* node = rn->getById(rn->getMapOfIdvsNodes() , id);
				if (node)
				{
					source = WayPoint(node);
				}
				break;
			}
			case 1:
			{
				srcType = "BS";
				sim_mob::BusStop* stop = sim_mob::BusStop::findBusStop(sSrc);
				if (stop)
				{
					source = WayPoint(stop);
				}
				break;
			}
			case 2:
			{
				srcType = "MS";
				sim_mob::TrainStop* stop = ptNetwork.findMRT_Stop(sSrc);
				if (stop)
				{
					source = WayPoint(stop);
				}
				break;
			}
			}

			if (source.type != WayPoint::INVALID && dest.type != WayPoint::INVALID)
			{
				subTrip.setPersonID(-1);
				subTrip.itemType = TripChainItem::getItemType("Trip");
				subTrip.sequenceNumber = 1;
				subTrip.startTime = curSubTrip->startTime;
				subTrip.endTime = DailyTime((*it).travelTime * 1000.0);
				subTrip.origin = source;
				subTrip.destination = dest;
				subTrip.startLocationId = sSrc;
				subTrip.startLocationType = srcType;
				subTrip.endLocationId = sEnd;
				subTrip.endLocationType = endType;
				subTrip.edgeId = (*it).id;
				subTrip.serviceLine = (*it).serviceLine;
				switch(source.type)
				{
				case WayPoint::BUS_STOP:
				case WayPoint::TRAIN_STOP:
					subTrip.originType = TripChainItem::LT_PUBLIC_TRANSIT_STOP;
					break;
				default:
					subTrip.originType = TripChainItem::LT_NODE;
					break;
				}

				switch(dest.type)
				{
				case WayPoint::BUS_STOP:
				case WayPoint::TRAIN_STOP:
					subTrip.destinationType = TripChainItem::LT_PUBLIC_TRANSIT_STOP;
					break;
				default:
					subTrip.destinationType = TripChainItem::LT_NODE;
					break;
				}

				subTrip.tripID = "";
				if ((*it).tType == sim_mob::WALK_EDGE)
				{
					subTrip.travelMode = "Walk";
					subTrip.isPT_Walk = true;
					subTrip.walkTime = (*it).walkTime;
				}
				else if ((*it).tType == sim_mob::BUS_EDGE)
				{
					subTrip.travelMode = "BusTravel";
				}
				else if((*it).tType == sim_mob::TRAIN_EDGE)
				{
					subTrip.travelMode = "MRT";
				}
				else if((*it).tType == sim_mob::SMS_EDGE)
				{
					subTrip.travelMode = "RAIL_SMS";
				}
				subTrip.ptLineId = it->serviceLines;
				newSubTrips.push_back(subTrip);
			}
			else
			{
				std::stringstream msg;
				msg << __func__ << ": Invalid PT path: Invalid Source/destination type for Person "
				    << personDbId << ", travelling from " << originNode.node->getNodeId()
				    << " to " << destNode.node->getNodeId() << " at time "
				    << (*currSubTrip).startTime.getStrRepr() << "\n";
				Warn() << msg.str();
				ret = false;
				break;
			}

			++it;
		}
	}
	return ret;
}

bool sim_mob::Person::updateNextSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*currTripChainItem);

	if (!trip)
	{
		return false;
	}

	if (currSubTrip == trip->getSubTrips().end())
	{
		return false;
	}

	nextSubTrip = currSubTrip + 1;

	if (nextSubTrip == trip->getSubTrips().end())
	{
		return false;
	}

	return true;
}


bool sim_mob::Person::updateNextTripChainItem()
{
	bool res = false;

	if (currTripChainItem == tripChain.end())
	{
		return false;
	}

	if ((*currTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		//check for next sub-trip first
		res = updateNextSubTrip();
	}

	if (res)
	{
		nextTripChainItem = currTripChainItem;
		return res;
	}

	//no, it is not the sub-trip we need to advance, it is the trip chain item
	nextTripChainItem = currTripChainItem + 1;
	if (nextTripChainItem == tripChain.end())
	{
		return false;
	}

	//Also set the nextSubTrip to the beginning of trip , just in case
	if ((*nextTripChainItem)->itemType == sim_mob::TripChainItem::IT_TRIP)
	{
		sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*nextTripChainItem); //easy reading
		if (!trip)
		{
			throw std::runtime_error("non sim_mob::Trip cannot have subtrips");
		}
		nextSubTrip = trip->getSubTrips().begin();
	}

	return true;
}

bool sim_mob::Person::advanceCurrentSubTrip()
{
	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*currTripChainItem);
	
	if (!trip)
	{
		return false;
	}

	if (currSubTrip == trip->getSubTrips().end()) /*just a routine check*/
	{
		return false;
	}

	//get metric
	/*TravelMetric currRoleMetrics;
	if(currRole != nullptr)
	{
		currRoleMetrics = currRole->Movement()->finalizeTravelTimeMetric();
		//serializeSubTripChainItemTravelTimeMetrics(*currRoleMetrics,currTripChainItem,currSubTrip);
	}*/

	++currSubTrip;

	if (currSubTrip == trip->getSubTrips().end())
	{
		return false;
	}
	return true;
}


bool sim_mob::Person::isTripValid()
{
	if (tripChain.empty())
	{
		return false;
	}

	if (currTripChainItem == tripChain.end())
	{
		return false;
	}

	sim_mob::Trip *trip = dynamic_cast<sim_mob::Trip *> (*currTripChainItem);

	if (!trip)
	{
		return false;
	}

	if (currSubTrip == trip->getSubTrips().end())
	{
		return false;
	}

	return true;
}

void sim_mob::Person::setPersonCharacteristics()
{
	const ConfigParams& config = ConfigManager::GetInstance().FullConfig();
	const std::map<int, PersonCharacteristics>& personCharacteristics = config.personCharacteristicsParams.personCharacteristics;

	int lowestAge = DEFAULT_LOWEST_AGE;
	int highestAge = DEFAULT_HIGHEST_AGE;

	// if no personCharacteristics item in the configuration file, introduce default lowestAge and highestAge
	if (!personCharacteristics.empty())
	{
		lowestAge = config.personCharacteristicsParams.lowestAge;
		highestAge = config.personCharacteristicsParams.highestAge;
	}

	boost::mt19937 gen(static_cast<unsigned int> (getId() * getId()));
	boost::uniform_int<> ageRange(lowestAge, highestAge);
	boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varAge(gen, ageRange);
	age = (unsigned int) varAge();	
}


/********************************************************************
 * ************* Collection and presentation of metrics *************
 * ******************************************************************
 */

namespace
{
sim_mob::OneTimeFlag titleSubPredayTT;
}

void sim_mob::Person::serializeSubTripChainItemTravelTimeMetrics(const TravelMetric& subtripMetrics, std::vector<TripChainItem*>::iterator currTripChainItem, 
																 std::vector<SubTrip>::iterator currSubTrip) const
{
	sim_mob::BasicLogger& csv = sim_mob::Logger::log(ConfigManager::GetInstance().FullConfig().subTripLevelTravelTimeOutput);
	if (!(subtripMetrics.finalized && subtripMetrics.started))
	{
		return;
	} //sanity check

	sim_mob::SubTrip &st = (*currSubTrip); //easy reading
	// restricted area which is to be appended at the end of the csv line
	std::stringstream restrictedRegion("");
	if (st.cbdTraverseType != sim_mob::TravelMetric::CBD_NONE)
	{
		//sanity check
		if (!(subtripMetrics.cbdOrigin.node && subtripMetrics.cbdDestination.node))
		{
			restrictedRegion <<
					subtripMetrics.origin.node->getNodeId() << "," <<
					subtripMetrics.destination.node->getNodeId() <<
					(st.cbdTraverseType == sim_mob::TravelMetric::CBD_ENTER ? " ,Enter" : " ,Exit") << " has null values " <<
					(subtripMetrics.cbdOrigin.node != nullptr ? subtripMetrics.cbdOrigin.node->getNodeId() : 0) << "," <<
					(subtripMetrics.cbdDestination.node != nullptr ? subtripMetrics.cbdDestination.node->getNodeId() : 0) << "\n";
		}
		else //valid scenario:
		{
			restrictedRegion <<
					subtripMetrics.cbdOrigin.node->getNodeId() << "," << //	cbd_entry_node
					subtripMetrics.cbdDestination.node->getNodeId() << "," << //	cbd_exit_node
					subtripMetrics.cbdStartTime.getStrRepr() << "," << //	cbd_entry_time
					subtripMetrics.cbdEndTime.getStrRepr() << "," << //	cbd_exit_time
					subtripMetrics.cbdTravelTime << "," << //	cbd_travel_time
					(subtripMetrics.travelTime - subtripMetrics.cbdTravelTime) << "," << //	non_cbd_travel_time
					subtripMetrics.cbdDistance << "," << //	cbd_distance
					(subtripMetrics.distance - subtripMetrics.cbdDistance); //	non_cbd_distance
		}
	}
	else// if Agent never entered or exited CBD
	{
		restrictedRegion <<
				"0" << "," << //	cbd_entry_node
				"0" << "," << //	cbd_exit_node
				"00:00:00" << "," << //	cbd_entry_time
				"00:00:00" << "," << //	cbd_exit_time
				"0" << "," << //	cbd_travel_time
				subtripMetrics.travelTime << "," << //	non_cbd_travel_time
				"0" << "," << //	cbd_distance
				subtripMetrics.distance; //	non_cbd_distance
	}

	std::string origin, destination;
	switch (subtripMetrics.origin.type)
	{
	case WayPoint::NODE:
	{
		origin = std::to_string(subtripMetrics.origin.node->getNodeId());
		break;
	}
	case WayPoint::BUS_STOP:
	{
		origin = subtripMetrics.origin.busStop->getStopCode();
		break;
	}
	case WayPoint::TRAIN_STOP:
	{
		const char* const delimiter = "/";
		std::ostringstream trainStopIdStrm;
		const std::vector<std::string>& trainStopIdVect = subtripMetrics.origin.trainStop->getTrainStopIds();
		std::copy(trainStopIdVect.begin(), trainStopIdVect.end(), std::ostream_iterator<std::string>(trainStopIdStrm, delimiter));
		origin = trainStopIdStrm.str();
		break;
	}

	case WayPoint::MRT_PLATFORM:
	{
		origin=subtripMetrics.origin.platform->getPlatformNo();
		break;
	}
	default:
	{
		origin = std::to_string(subtripMetrics.origin.type);
		break;
	}
	}

	switch (subtripMetrics.destination.type)
	{
	case WayPoint::NODE:
	{
		destination = std::to_string(subtripMetrics.destination.node->getNodeId());
		break;
	}
	case WayPoint::BUS_STOP:
	{
		destination = subtripMetrics.destination.busStop->getStopCode();
		break;
	}
	case WayPoint::TRAIN_STOP:
	{
		const char* const delimiter = "/";
		std::ostringstream trainStopIdStrm;
		const std::vector<std::string>& trainStopIdVect = subtripMetrics.destination.trainStop->getTrainStopIds();
		std::copy(trainStopIdVect.begin(), trainStopIdVect.end(), std::ostream_iterator<std::string>(trainStopIdStrm, delimiter));
		destination = trainStopIdStrm.str();
		break;
	}

	case WayPoint::MRT_PLATFORM:
	{
		destination=subtripMetrics.destination.platform->getPlatformNo();
		break;
	}
	default:
	{
		destination = std::to_string(subtripMetrics.destination.type);
		break;
	}
	}

	std::stringstream res("");
	// actual writing
	//note: Even though we output the travel time only for the subtrip, preday expects the PT in-vehicle time for the original Trip.
	//      We therefore output the origin and destination Zone loaded from the DAS for the current trip.
	//      The zones of origiNode and destNode are irrelevant.
	const Trip *trip = (static_cast<Trip*> (*currTripChainItem));
	res <<
			trip->getPersonID() << "," << //	person_id
			trip->getPersonID() << "_" << trip->sequenceNumber << "," << //	trip_id
			st.tripID << "," << //	subtrip_id
			origin << "," << //	origin
			(*currTripChainItem)->originZoneCode << "," <<
			destination << "," << //	destination
			(*currTripChainItem)->destinationZoneCode << "," <<
			st.travelMode << "," << //	mode
			subtripMetrics.startTime.getStrRepr() << "," << //	start_time
			subtripMetrics.endTime.getStrRepr() << "," << //	end_time
			subtripMetrics.travelTime << "," << //	travel_time
			subtripMetrics.distance << "," << //	total_distance
			restrictedRegion.str() << "\n"; /* MIXED CBD Information */

	csv << res.str();


	if (st.travelMode == "\"Car\"" || st.travelMode == "Car" )                //Collecting Data for travel_time (Car_Travel_time.csv)for
	{                                              //passenger travelling in own "CAR" (RL_DRIVER)
		std::stringstream result_car("");
		result_car << trip->getPersonID() << ","   //person_id
		           << trip->startLocationId << "," //trip_origin_id
		           << trip->endLocationId << ","   //trip_dest_id
		           << trip->startLocationId << ","    //subtrip_origin_id
		           << trip->endLocationId << ","      //subtrip_dest_id
		           << "NODE" << ","                //subtrip_origin_type
		           << "NODE" << ","                //subtrip_dest_type
		           << "ON_CAR" << ","              //travel_mode
		           << subtripMetrics.startTime.getStrRepr() << ","    //arrival_time
		           << subtripMetrics.travelTime * 3600 << ","  //travel_time = arrival_time-end_time
		<<(st.ptLineId.empty() ? "\"\"" : st.ptLineId)<<std::endl;  //pt_line
		CarTravelTimeLogger << result_car.str();

	}
	int cbdStartNode = 0, cbdEndNode = 0;
	if (subtripMetrics.cbdOrigin.type == WayPoint::NODE)
	{
		cbdStartNode = subtripMetrics.cbdOrigin.node->getNodeId();
	}
	if (subtripMetrics.cbdDestination.type == WayPoint::NODE)
	{
		cbdEndNode = subtripMetrics.cbdDestination.node->getNodeId();
	}
	if (subtripMetrics.cbdTraverseType == TravelMetric::CBD_NONE || cbdStartNode == 0 || cbdEndNode == 0)
	{
		return;
	}
	std::stringstream ret("");
	ret << this->getId() << "," << origin << "," << destination << ","
			<< cbdStartNode << "," << cbdEndNode << ","
			<< subtripMetrics.cbdStartTime.getStrRepr() << ","
			<< subtripMetrics.cbdEndTime.getStrRepr() << ","
			<< st.travelMode << ","
			<< subtripMetrics.cbdTraverseType << std::endl;
	sim_mob::BasicLogger& cbd = sim_mob::Logger::log("cbd.csv");
	cbd << ret.str();
}

void sim_mob::Person::initTripChain()
{
}



