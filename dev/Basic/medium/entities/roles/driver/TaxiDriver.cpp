/*
 * TaxiDriver.cpp
 *
 *  Created on: 5 Nov 2016
 *      Author: jabir
 */

#include <entities/roles/driver/TaxiDriver.hpp>
#include "path/PathSetManager.hpp"

namespace sim_mob
{
	namespace medium
	{

	 	void TaxiDriver::addPassenger(Passenger *passenger)
	 	{
	 		taxiPassenger = passenger;
	 	}

	 	Passenger * TaxiDriver::alightPassenger()
	 	{
	 		Passenger *pGr = taxiPassenger;
	 		taxiPassenger = nullptr;
	 		return pGr;
	 	}

	 	void TaxiDriver::boardPassenger(Passenger *passenger)
	 	{
	 		const Person *person = passenger->getParent();
	 		//the person decides the route choice
	 		SubTrip currSubTrip;
	 		bool useInSimulationTT = person->usesInSimulationTravelTime();
	 		std::vector<WayPoint> travelPath = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);
	 		currentRouteChoice = travelPath;
	 		taxiPassenger = passenger;
	 	}

	 	void TaxiDriver::runRouteChoiceModel(Node *origin,Node *destination)
	 	{
	 		std::vector<WayPoint> res;
	 		bool useInSimulationTT = parent->usesInSimulationTravelTime();
	 		SubTrip currSubTrip;
	 		currSubTrip.origin = WayPoint(origin);
	 		currSubTrip.destination = WayPoint(destination);
	 		currentRouteChoice = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);
	 	}

	 	void TaxiDriver::driveToDestinationNode(Node *destinationNode)
	 	{
	 		runRouteChoiceModel(currentNode,destinationNode);

	 	}

	 	void TaxiDriver::getLinkAndRoadSegments(Node * start ,Node *end,std::vector<RoadSegment*>& segments)
	 	{
	 		std::map<unsigned int,Link*> downStreamLinks = start->getDownStreamLinks();
	 		std::map<unsigned int,Link*>::iterator itr = downStreamLinks.begin();
	 		while(itr!=downStreamLinks.end())
	 		{
	 			Link *link=itr->second;
	 			Node *toNode = link->getToNode();
	 			if(toNode == end)
	 			{
	 				segments = link->getRoadSegments();
	 				break;
	 			}
	 			itr++;
	 		}
	 	}

	 	void TaxiDriver::setCruisingMode()
	 	{
	 		driverMode = CRUISE;
	 		std::vector<Node*> nodeVector = currentNode->getNeighbouringNodes();
	 		if(nodeVector.size()>1)
	 		{
	 			std::vector<Node*>::iterator itr = nodeVector.begin();
	 			Node *destNode = (*(itr+1));
	 			Node *originNode = *itr;
	 			std::map<unsigned int,Link*> mapOfDownStreamLinks = originNode->getDownStreamLinks();
	 			std::map<unsigned int,Link*>::iterator linkItr = mapOfDownStreamLinks.begin();
	 			while(linkItr!=mapOfDownStreamLinks.end())
	 			{
	 				Link *link = (*linkItr).second;
	 				Node *toNode = link->getToNode();
	 				if(toNode == destNode)
	 				{
	 					currentRoute = link->getRoadSegments();
	 				}
	 				linkItr++;
	 			}
	 		}
	 		//depth first or breath first
	 		//select a node
	 	}

	 	void TaxiDriver::setDriveMode(DriverMode mode)
	 	{
	 		driverMode = mode;
	 	}

	 	Person *TaxiDriver::getParent()
	 	{
	 		return parent;
	 	}

	 	void TaxiDriver::driveToTaxiStand(/*TaxiStand *stand */)
	 	{
	 		driverMode = DRIVE_TO_TAXISTAND;
	 		SubTrip currSubTrip;
	 		//currSubTrip.origin = assign node or segment
	 		bool useInSimulationTT = parent->usesInSimulationTravelTime();
	 		RoadSegment *taxiStandSegment;
	 		//RoadSegment taxiStandSegment = TaxiStand->getSegment();
	 		const Link *taxiStandLink = taxiStandSegment->getParentLink();
	 		Node *destForRouteChoice = taxiStandLink->getFromNode();
	 		//get the current segment
	 		const Link *currSegmentParentLink = currSegment->getParentLink();
	 		Node *originNode = currSegmentParentLink->getToNode();
	 		//set origin and destination node
	 		currSubTrip.origin = WayPoint(originNode);
	 		currSubTrip.destination = WayPoint(destForRouteChoice);
	 		PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);

	 	}


	 	void TaxiDriver::driveToNode(Node *destinationNode)
	 	{
	 		const Link *currSegmentParentLink = currSegment->getParentLink();
	 		Node *originNode = currSegmentParentLink->getToNode();
	 		SubTrip currSubTrip;
	 		currSubTrip.origin = WayPoint(originNode);
	 		currSubTrip.destination = WayPoint(destinationNode);
	 		bool useInSimulationTT = parent->usesInSimulationTravelTime();
	 		std::vector<WayPoint> currentRoute = PrivateTrafficRouteChoice::getInstance()->getPath(currSubTrip,false, nullptr, useInSimulationTT);
	 	}

	 	void TaxiDriver::checkPersonsAndPickUpAtNode(timeslice now)
	 	{
	 		//pick up the first person from the node and advance its trip chain
	 		std::vector<Person*> personsWaiting = currentNode->personsWaitingForTaxi();
	 		if(personsWaiting.size()>0)
	 		{
				Person *personToPickUp = personsWaiting[0];
				DailyTime current(DailyTime(now.ms()));//.offsetMS_From(ConfigManager::GetInstance().FullConfig().simStartTime()));
				Person_MT *personToPickUp_MT = dynamic_cast<Person_MT*>(personToPickUp);
				personToPickUp->checkTripChain(current.getValue());
				Role<Person_MT>* curRole = personToPickUp_MT->getRole();
				curRole->setArrivalTime(now.ms());
				sim_mob::medium::Passenger* passenger = dynamic_cast<sim_mob::medium::Passenger*>(curRole);
				if (passenger)
				{
					addPassenger(passenger);
					passenger->setStartPoint(WayPoint(currentNode));
					//run the route choice and pass the route to taxi driver
					passenger->Movement()->startTravelTimeMetric();
				}
	 		}
	 	}

	 	bool TaxiDriver::hasPersonBoarded()
	 	{
	 		return personBoarded;
	 	}

	 	void TaxiDriver::setCurrentNode(Node *currNode)
	 	{
	 		currentNode = currNode;
	 	}

	 	void TaxiDriver::setDestinationNode(Node *destinationNode)
	 	{
	 		this->destinationNode = destinationNode;
	 	}

	 	Node * TaxiDriver::getDestinationNode()
	 	{
	 		return destinationNode;
	 	}

	 	Node * TaxiDriver::getCurrentNode()
	 	{
	 		return currentNode;
	 	}

		TaxiDriver::~TaxiDriver()
		{
			// TODO Auto-generated destructor stub
		}
	}
}

