/*
 * PedestrainFacets.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: zhang huai peng
 */

#include "PedestrianFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "config/MT_Config.hpp"
#include "logging/ControllerLog.hpp"
#include "message/MessageBus.hpp"
#include "Pedestrian.hpp"
#include "util/Utils.hpp"

using namespace sim_mob;
using namespace sim_mob::medium;

PedestrianBehavior::PedestrianBehavior() : BehaviorFacet(), parentPedestrian(nullptr)
{
}

PedestrianBehavior::~PedestrianBehavior()
{
}

PedestrianMovement::PedestrianMovement(double speed) :
		MovementFacet(), parentPedestrian(nullptr), walkSpeed(speed), destinationNode(nullptr), totalTimeToCompleteSec(10),
		secondsInTick(ConfigManager::GetInstance().FullConfig().baseGranSecond())
{
}

PedestrianMovement::~PedestrianMovement()
{
}

void PedestrianMovement::setParentPedestrian(medium::Pedestrian* parentPedestrian)
{
	this->parentPedestrian = parentPedestrian;
}

TravelMetric& PedestrianMovement::startTravelTimeMetric()
{
	return travelMetric;
}

TravelMetric& PedestrianMovement::finalizeTravelTimeMetric()
{
	return travelMetric;
}

void PedestrianBehavior::setParentPedestrian(medium::Pedestrian* parentPedestrian)
{
	this->parentPedestrian = parentPedestrian;
}

void PedestrianMovement::frame_init()
{
	destinationNode = getDestNode();

	if(!destinationNode)
	{
		throw std::runtime_error("destination segment not found");
	}

	SubTrip& subTrip = *(parentPedestrian->parent->currSubTrip);
	double walkTime = 0.0;

	if(subTrip.isPT_Walk)
	{
		walkTime = subTrip.walkTime; //walk time comes from db for PT pedestrians
	}
	else if(subTrip.isTT_Walk)
	{
		isOnDemandTraveler = false;
		Person_MT *person = parentPedestrian->getParent();

		if (subTrip.origin.type == WayPoint::NODE && subTrip.destination.type == WayPoint::NODE)
		{
			isOnDemandTraveler = true;
			const unsigned int taxiStartNodeId = subTrip.destination.node->getNodeId();

			if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
			{
				std::vector<SubTrip>::iterator subTripItr = person->currSubTrip;

				if ((*subTripItr).travelMode == "TravelPedestrian" && subTrip.origin.node == subTrip.destination.node)
				{
					std::vector<SubTrip>::iterator taxiTripItr = subTripItr + 1;
					const unsigned int taxiEndNodeId = (*taxiTripItr).destination.node->getNodeId();


					auto controllers = MobilityServiceControllerManager::GetInstance()->getControllers();
					unsigned int randomController = Utils::generateInt(0, controllers.size() - 1);
					auto itControllers = controllers.begin();
					advance(itControllers, randomController);

					TripRequestMessage* request = new TripRequestMessage(person->currTick,
												  person->getDatabaseId(),
												  taxiStartNodeId, taxiEndNodeId, 0);
					messaging::MessageBus::SendMessage(itControllers->second, MSG_TRIP_REQUEST,
					                                   messaging::MessageBus::MessagePtr(
							                                   request));


					ControllerLog() << "Request sent: "<< *request << std::endl;
				}
			}
		}

		if(!isOnDemandTraveler)
		{
			const Node* source = subTrip.origin.node;
			const TaxiStand* stand = subTrip.destination.taxiStand;
			const Node* destination = stand->getRoadSegment()->getParentLink()->getFromNode();
			std::vector<WayPoint> path = StreetDirectory::Instance().SearchShortestDrivingPath<Node, Node>(*(source), *(destination));
			for (auto itWayPts = path.begin(); itWayPts != path.end(); ++itWayPts)
			{
				if (itWayPts->type == WayPoint::LINK)
				{
					TravelTimeAtNode item;
					item.node = itWayPts->link->getToNode();
					item.travelTime = itWayPts->link->getLength() / walkSpeed;
					travelPath.push(item);
				}
			}
		}

		Conflux* startConflux = this->getStartConflux();

		if (startConflux)
		{
			messaging::MessageBus::PostMessage(startConflux, MSG_TRAVELER_TRANSFER,
			                                   messaging::MessageBus::MessagePtr(new PersonMessage(person)));
		}
	}
	else // both origin and destination must be nodes
	{
		if(subTrip.origin.type != WayPoint::NODE || subTrip.destination.type != WayPoint::NODE)
		{
			throw std::runtime_error("non node O/D for not PT pedestrian");
		}
		const Node* srcNode = subTrip.origin.node;
		const Node* destNode = subTrip.destination.node;

		DynamicVector distVector(srcNode->getLocation().getX(),srcNode->getLocation().getY(),destNode->getLocation().getX(),destNode->getLocation().getY());
		double distance = distVector.getMagnitude();
		walkTime = distance / walkSpeed;
	}

	parentPedestrian->setTravelTime(walkTime*1000);
}

const Node* PedestrianMovement::getDestNode()
{
	SubTrip& subTrip = *(parentPedestrian->parent->currSubTrip);
	const Node* destNd = nullptr;

	switch(subTrip.destination.type)
	{
	case WayPoint::NODE:
	{
		destNd = subTrip.destination.node;
		break;
	}
	case WayPoint::TRAIN_STOP:
	{
		const Node* srcNode = nullptr;
		switch(subTrip.origin.type)
		{
		case WayPoint::NODE:
		{
			srcNode = subTrip.origin.node;
			break;
		}
		case WayPoint::BUS_STOP:
		{
			srcNode = subTrip.origin.busStop->getParentSegment()->getParentLink()->getFromNode();
			break;
		}
		case WayPoint::TRAIN_STOP:
		{
			//this case should ideally not occur. handling just in case...
			srcNode = subTrip.origin.trainStop->getRandomStationSegment()->getParentLink()->getFromNode();
			break;
		}
		}
		destNd = subTrip.destination.trainStop->getStationSegmentForNode(srcNode)->getParentLink()->getToNode();
		break;
	}
	case WayPoint::BUS_STOP:
	{
		destNd = subTrip.destination.busStop->getParentSegment()->getParentLink()->getToNode();
		break;
	}
	case WayPoint::TAXI_STAND:
	{
		destNd = subTrip.destination.taxiStand->getRoadSegment()->getParentLink()->getFromNode();
		break;
	}
	}
	return destNd;
}

void PedestrianMovement::frame_tick()
{
	parentPedestrian->parent->setRemainingTimeThisTick(0);

	if (parentPedestrian->roleType == Role<Person_MT>::RL_TRAVELPEDESTRIAN)
	{
		unsigned int tickMS = ConfigManager::GetInstance().FullConfig().baseGranMS();
		parentPedestrian->setTravelTime(parentPedestrian->getTravelTime()+tickMS);
		double tickSec = ConfigManager::GetInstance().FullConfig().baseGranSecond();

		if (!isOnDemandTraveler)
		{
			TravelTimeAtNode& front = travelPath.front();
			if (front.travelTime < tickSec)
			{
				travelPath.pop();
				Conflux* start = this->getStartConflux();
				if (start)
				{
					messaging::MessageBus::PostMessage(start, MSG_TRAVELER_TRANSFER,
							messaging::MessageBus::MessagePtr(new PersonMessage(parentPedestrian->parent)));
				}
			}
			else
			{
				front.travelTime -= tickSec;
			}
			if (travelPath.size() == 0)
			{
				parentPedestrian->parent->setToBeRemoved();
			}
		}
		else
		{
			parentPedestrian->getParent()->setToBeRemoved();
		}
	}
}

std::string PedestrianMovement::frame_tick_output()
{
	return std::string();
}

Conflux* PedestrianMovement::getStartConflux() const
{
	if (parentPedestrian->roleType == Role<Person_MT>::RL_TRAVELPEDESTRIAN && travelPath.size() > 0)
	{
		const TravelTimeAtNode &front = travelPath.front();
		return MT_Config::getInstance().getConfluxForNode(front.node);
	}
	else if (parentPedestrian->roleType == Role<Person_MT>::RL_TRAVELPEDESTRIAN && isOnDemandTraveler)
	{
		SubTrip &subTrip = *(parentPedestrian->parent->currSubTrip);
		return MT_Config::getInstance().getConfluxForNode(subTrip.origin.node);
	}

	return nullptr;
}

Conflux* PedestrianMovement::getDestinationConflux() const
{
	if (destinationNode)
	{
		return MT_Config::getInstance().getConfluxForNode(destinationNode);
	}
	return nullptr;
}

bool PedestrianMovement::getOnDemandTraveller()
{
	return isOnDemandTraveler;
}
