//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Pedestrian.hpp"
#include "PedestrianFacets.hpp"
#include "entities/Person.hpp"
#include "entities/roles/Role.hpp"
#include "entities/Person_ST.hpp"
#include "conf/ConfigManager.hpp"
#include "util/Utils.hpp"
#include "config/ST_Config.hpp"
#include "entities/controllers/MobilityServiceController.hpp"
#include "message/MessageBus.hpp"

using namespace std;
using namespace sim_mob;
using namespace messaging;

PedestrianMovement::PedestrianMovement() :
        MovementFacet(),parentPedestrian(nullptr), distanceToBeCovered(0)
{
}

PedestrianMovement::~PedestrianMovement()
{
}
void PedestrianMovement::frame_init()
{
	//Extract the origin and destination from the sub-trip
	
	SubTrip &subTrip = *(parentPedestrian->parent->currSubTrip);
	
	const Node *originNode = nullptr;
	const Point *origin = nullptr;
	const Point *destination = nullptr;
    const Node *destinationNode = nullptr;

	switch(subTrip.origin.type)
	{
	case WayPoint::NODE: 
		originNode = subTrip.origin.node;
		origin = &(originNode->getLocation());
		break;
		
	case WayPoint::BUS_STOP:
		originNode = subTrip.origin.busStop->getParentSegment()->getParentLink()->getFromNode();
		origin = &(subTrip.origin.busStop->getStopLocation());
		break;
		
	case WayPoint::TRAIN_STOP:
		originNode = subTrip.origin.trainStop->getRandomStationSegment()->getParentLink()->getFromNode();
		origin = &(originNode->getLocation());
		break;
		
	default:
		stringstream msg;
		msg << __func__ << ": Origin type for pedestrian is invalid!\n";
		msg << "Type: " << subTrip.origin.type << " Person id: " << parentPedestrian->parent->getDatabaseId();
		throw runtime_error(msg.str());
	}
	
	switch(subTrip.destination.type)
	{
	case WayPoint::NODE: 
		destination = &(subTrip.destination.node->getLocation());
		break;
		
	case WayPoint::BUS_STOP:
		destination = &(subTrip.destination.busStop->getStopLocation());
		break;
		
	case WayPoint::TRAIN_STOP:
		destination = &(subTrip.destination.trainStop->getStationSegmentForNode(originNode)->getParentLink()->getToNode()->getLocation());
		break;

    case WayPoint::TAXI_STAND:
    {
        destinationNode = subTrip.destination.taxiStand->getRoadSegment()->getParentLink()->getFromNode();
        break;
    }
    default:
		stringstream msg;
		msg << __func__ << ": Destination type for pedestrian is invalid!\n";
		msg << "Type: " << subTrip.destination.type << " Person id: " << parentPedestrian->parent->getDatabaseId();
		throw runtime_error(msg.str());
	}
	//Get the distance between the origin and destination	
	DynamicVector distVector(*origin, *destination);
	distanceToBeCovered = distVector.getMagnitude();
	
	//Set the travel time in milli-seconds
	parentPedestrian->setTravelTime((distanceToBeCovered / parentPedestrian->parent->getWalkingSpeed()) * 1000);

    // Send trip request message to the controller
    Person_ST *person = parentPedestrian->getParent();

    if (subTrip.origin.type == WayPoint::NODE && subTrip.destination.type == WayPoint::NODE)
    {
        const Node *taxiStartNode = subTrip.destination.node;

        if (MobilityServiceControllerManager::HasMobilityServiceControllerManager())
        {
            std::vector<SubTrip>::iterator subTripItr = person->currSubTrip;

            if ((*subTripItr).travelMode == "TravelPedestrian" && subTrip.origin.node == subTrip.destination.node)
            {
                std::vector<SubTrip>::iterator taxiTripItr = subTripItr + 1;
                const Node *taxiEndNode = (*taxiTripItr).destination.node;

            //choose the controller based on the mode
                auto controllers = MobilityServiceControllerManager::GetInstance()->getControllers();
                MobilityServiceController *controller = nullptr;
                if((*taxiTripItr).travelMode.find("AMOD") != std::string::npos)
                {
                    //If the person is taking an AMOD service, get the AMOD controller
                    auto itCtrlr = controllers.get<ctrlrType>().find(SERVICE_CONTROLLER_AMOD);
                    if (itCtrlr == controllers.get<ctrlrType>().end())
                    {
                        std::stringstream msg;
                        msg << "Controller of type " << toString(SERVICE_CONTROLLER_AMOD)
                            << " has not been added, but "
                            << "the demand contains persons taking AMOD service";
                            throw std::runtime_error(msg.str());
                    }
                    controller = *itCtrlr;
                }
                else
                {
                    //Choose randomly from available controllers
                    const ConfigParams &cfg = ConfigManager::GetInstance().FullConfig();
                    auto enabledCtrlrs = cfg.mobilityServiceController.enabledControllers;
                    auto it = enabledCtrlrs.begin();
                    auto randomNum = Utils::generateInt(0, enabledCtrlrs.size() - 1);
                    std::advance(it, randomNum);
                    //Here we have to search by id, as the enabled controllers map has id as the key
                    auto itCtrlr = controllers.get<ctrlrId>().find(it->first);
                    controller = *itCtrlr;
                }

#ifndef NDEBUG
                consistencyChecks(controller->getServiceType());
                controller->consistencyChecks();

#endif
                //If the the request is a pool request, set type as shared. Else it is a single request
                RequestType reqType = RequestType::TRIP_REQUEST_SINGLE;
                if((*taxiTripItr).travelMode.find("Pool") != std::string::npos)
                {

                    reqType = RequestType::TRIP_REQUEST_SHARED;

                }
                TripRequestMessage* request = new TripRequestMessage(person->currTick,person,
                                                                     person->getDatabaseId(),
                                                                     taxiStartNode, taxiEndNode, MobilityServiceController::toleratedExtraTime,reqType);

                MessageBus::PostMessage(controller, MSG_TRIP_REQUEST, MessageBus::MessagePtr(request));


                ControllerLog() << "Request sent to controller of type "<< toString(controller->getServiceType()) << ": ID : "<< controller->getControllerId()<<": "<< *request << std::endl;
            }
        }
    }

}

void PedestrianMovement::frame_tick()
{
	double elapsedTime = ConfigManager::GetInstance().FullConfig().baseGranSecond();
	double distanceCovered = parentPedestrian->parent->getWalkingSpeed() * elapsedTime;
	distanceToBeCovered -= distanceCovered;
	
	if(distanceToBeCovered <= 0)
	{
		parentPedestrian->getParent()->setToBeRemoved();
	}
}

std::string PedestrianMovement::frame_tick_output()
{
	return std::string();
}


