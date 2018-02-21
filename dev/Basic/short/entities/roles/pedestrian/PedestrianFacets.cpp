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
                TripChainItem *tcItem = *(person->currTripChainItem);

                //Choose the controller based on the stop mode in das (i.e. SMS/SMS_POOL,AMOD,RAIL_SMS,etc..)
                auto controllers = MobilityServiceControllerManager::GetInstance()->getControllers();
                MobilityServiceController *controller = nullptr;
                const ConfigParams &cfg = ConfigManager::GetInstance().FullConfig();
                auto enabledCtrlrs = cfg.mobilityServiceController.enabledControllers;
                std::map<unsigned int, MobilityServiceControllerConfig>:: iterator itr ;
                for (itr = enabledCtrlrs.begin(); itr != enabledCtrlrs.end(); itr++)
                {
                	std::string currentTripChainMode = boost::to_upper_copy(tcItem->getMode());
                    if (boost::to_upper_copy(itr->second.tripSupportMode).find(currentTripChainMode.insert(0,"|").append("|"))!= std::string::npos)
                    {
                    	auto itCtrlr = controllers.get<ctrlTripSupportMode>().find(itr->second.tripSupportMode);
                    	controller = *itCtrlr;
                    	break;
                    }
                }
                if (!controller)
                {
                    std::stringstream msg;
                    msg << "Controller for person travelmode " <<tcItem->getMode()
                        << " is not present in config file,while the demand (DAS) have this mode";
                    throw std::runtime_error(msg.str());
                }

#ifndef NDEBUG
                consistencyChecks(controller->getServiceType());
                controller->consistencyChecks();

#endif
                //If the the request is a pool request, set type as shared. Else it is a single request
                RequestType reqType = RequestType::TRIP_REQUEST_SINGLE;
                if((*tcItem).travelMode.find("Pool") != std::string::npos)
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


