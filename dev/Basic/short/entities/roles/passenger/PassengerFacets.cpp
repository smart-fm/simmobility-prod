//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "PassengerFacets.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Person.hpp"
#include "entities/BusStopAgent.hpp"
#include "geospatial/Link.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "logging/Log.hpp"
#include "entities/Person_ST.hpp"

using namespace sim_mob;

namespace sim_mob
{

BusStop* getBusStop(const Node* node, sim_mob::RoadSegment* segment)
{
	std::map<centimeter_t, const RoadItem*>::const_iterator ob_it;
	const std::map<centimeter_t, const RoadItem*> & obstacles = segment->obstacles;
	for (ob_it = obstacles.begin(); ob_it != obstacles.end(); ++ob_it)
	{
		RoadItem* ri = const_cast<RoadItem*> (ob_it->second);
		BusStop *bs = dynamic_cast<BusStop*> (ri);
		if (bs && ((segment->getStart() == node) || (segment->getEnd() == node)))
		{
			return bs;
		}
	}

	return nullptr;
}

PassengerBehavior::PassengerBehavior() :
BehaviorFacet(), parentPassenger(nullptr)
{
}

PassengerBehavior::~PassengerBehavior()
{
}

void PassengerBehavior::frame_init()
{
	throw std::runtime_error("PassengerBehavior::frame_init is not implemented yet");
}

void PassengerBehavior::frame_tick()
{
	throw std::runtime_error("PassengerBehavior::frame_tick is not implemented yet");
}

void PassengerBehavior::frame_tick_output()
{
	throw std::runtime_error("PassengerBehavior::frame_tick_output is not implemented yet");
}

sim_mob::PassengerMovement::PassengerMovement() :
MovementFacet(), parentPassenger(nullptr), alightingMS(0),
waitingTime(-1), timeOfReachingBusStop(0), displayX(0), displayY(0), skip(0),
timeOfStartTrip(0), travelTime(0), busTripRunNum(0), buslineId("")
{
}

sim_mob::PassengerMovement::~PassengerMovement()
{

}

void sim_mob::PassengerMovement::setParentBufferedData()
{
	if (parentPassenger->busdriver.get() != nullptr)
	{
		parentPassenger->parent->xPos.set(parentPassenger->busdriver.get()->getCurrPosition().x);
		parentPassenger->parent->yPos.set(parentPassenger->busdriver.get()->getCurrPosition().y);
	}
}

void sim_mob::PassengerMovement::frame_init()
{
	//initialization
	//	WaitingTime = -1;
	if (parentPassenger->parent->originNode.type_ == WayPoint::BUS_STOP && parentPassenger->parent->destNode.type_ == WayPoint::BUS_STOP)
	{
		BusStopAgent* originBusstopAg = BusStopAgent::findBusStopAgentByBusStopNo(parentPassenger->parent->originNode.busStop_->getBusstopno_());
		parentPassenger->parent->xPos.force(originBusstopAg->getBusStop().xPos); // set xPos to WaitBusActivityRole
		parentPassenger->parent->yPos.force(originBusstopAg->getBusStop().yPos); // set yPos to WaitBusActivityRole
		originBusStop = const_cast<BusStop*> (parentPassenger->parent->originNode.busStop_);
		destBusStop = const_cast<BusStop*> (parentPassenger->parent->destNode.busStop_);
		timeOfStartTrip = parentPassenger->parent->currTick.ms();
		if (parentPassenger->parent)
		{
			parentPassenger->parent->setNextRole(nullptr); // set nextRole to be nullptr at frame_init
		}
		return;
	}
	originBusStop = nullptr;
	if (parentPassenger->parent->originNode.type_ == WayPoint::NODE)
	{
		originBusStop = setBusStopXY(parentPassenger->parent->originNode.node_);
	}

	if (originBusStop)
	{
		parentPassenger->parent->xPos.set(originBusStop->xPos);
		parentPassenger->parent->yPos.set(originBusStop->yPos);
	}
	else if (parentPassenger->parent->originNode.type_ == WayPoint::NODE)
	{
		const Node* node = parentPassenger->parent->originNode.node_;
		parentPassenger->parent->xPos.set(node->getLocation().getX());
		parentPassenger->parent->yPos.set(node->getLocation().getY());
	}

	destBusStop = nullptr;
	if (parentPassenger->parent->destNode.type_ == WayPoint::NODE)
	{
		destBusStop = setBusStopXY(parentPassenger->parent->destNode.node_);
	}

	timeOfStartTrip = parentPassenger->parent->currTick.ms();
	;
	if (parentPassenger->parent)
	{
		parentPassenger->parent->setNextRole(nullptr); // set nextRole to be nullptr at frame_init
	}
	FindBusLines(); //to find which bus lines the passenger wants to board based on busline info at busstop
}

void sim_mob::PassengerMovement::frame_tick()
{
	PassengerUpdateParams &p = parentPassenger->getParams();
	if (0 != alightingMS)
	{
		if (alightingMS == p.now.ms())
		{
			alightingMS = 0;
			//Person* person = dynamic_cast<Person*> (parent);
			if (parentPassenger->parent)
			{
				if (!parentPassenger->parent->findPersonNextRole())// find and assign the nextRole to this Person, when this nextRole is set to be nullptr?
				{
					std::cout << "End of trip chain...." << std::endl;
				}
				Passenger* passenger = dynamic_cast<Passenger*> (parentPassenger->parent->getNextRole());
				if (passenger)
				{// nextRole is passenger, create temporary role to avoid tripchain
					const RoleFactory<Person_ST> *rf = RoleFactory<Person_ST>::getInstance();
					Role<Person_ST> *newRole = rf->createRole("waitBusActivityRole", parentPassenger->parent);
					parentPassenger->parent->changeRole(newRole);
					newRole->Movement()->frame_init();
				}
				else
				{
					parentPassenger->parent->setToBeRemoved(); //removes passenger if destination is reached
					travelTime = p.now.ms() - parentPassenger->parent->getAlightingCharacteristics() * 1000 - timeOfStartTrip;
					const uint32_t waitingTimeAtStop = parentPassenger->getWaitingTimeAtStop();
					
					PassengerInfoPrint() << "iamwaiting id " << parentPassenger->parent->getId() 
							<< " from " << parentPassenger->parent->originNode.busStop_->busstopno_ 
							<< " to " << parentPassenger->parent->destNode.busStop_->busstopno_ << " "
							<< (ConfigManager::GetInstance().FullConfig().simStartTime() + DailyTime(parentPassenger->parent->getStartTime()) + DailyTime(waitingTimeAtStop)).getStrRepr() << " "
							<< waitingTimeAtStop << " bustripRunNum " << getBusTripRunNum() << " buslineid " << getBuslineId() 
							<< " TravelTime " << travelTime << std::endl;
					
					parentPassenger->busdriver.set(nullptr); // assign this busdriver to Passenger
					parentPassenger->BoardedBus.set(false);
					parentPassenger->AlightedBus.set(true);
				}
			}
		}
		else
		{
			setParentBufferedData(); //update passenger coordinates every frame tick
		}
	}
}

void sim_mob::PassengerMovement::frame_tick_output()
{
	PassengerUpdateParams &p = parentPassenger->getParams();
	//Reset our offset if it's set to zero
	if (displayOffset.getX() == 0 && displayOffset.getY() == 0)
	{
		boost::mt19937 gen(static_cast<unsigned int> (parentPassenger->parent->getId() * parentPassenger->parent->getId()));
		boost::uniform_int<> distX(0, 249);
		boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varX(gen, distX);
		boost::uniform_int<> distY(0, 99);
		boost::variate_generator < boost::mt19937, boost::uniform_int<int> > varY(gen, distY);
		unsigned int value = (unsigned int) varX();
		displayOffset.setX(value + 1);
		value = (unsigned int) varY();
		displayOffset.setY(value + 1);
	}

	//MPI-specific output.
	std::stringstream addLine;
	if (ConfigManager::GetInstance().FullConfig().using_MPI)
	{
		addLine << "\",\"fake\":\"" << (this->parentPassenger->parent->isFake ? "true" : "false");
	}

	//Figure out the (x,y) position of the passenger; differs if they are waiting on the side of the road versus
	// riding on a bus.
	int xPos = 0;
	int yPos = 0;


	if ((parentPassenger->BoardedBus.get() == false) && (parentPassenger->AlightedBus.get() == false))
	{
		//output passenger on visualizer only if passenger on road
		xPos = parentPassenger->parent->xPos.get() + displayOffset.getX();
		yPos = parentPassenger->parent->yPos.get() + displayOffset.getY();
	}
	else if ((parentPassenger->BoardedBus.get() == false) && (parentPassenger->AlightedBus.get() == true))
	{
		//output passenger on visualizer only if passenger on road
		xPos = displayX - displayOffset.getX() - displayOffset.getX();
		yPos = displayY - displayOffset.getY() - displayOffset.getY();
	}

	LogOut("(\"passenger"
		<< "\"," << parent->currTick.frame()
		<< "," << parent->getId()
		<< "," << "{\"xPos\":\"" << xPos
		<< "\"," << "\"yPos\":\"" << yPos
		<< addLine.str()
		<< "\",})" << std::endl);
}

bool sim_mob::PassengerMovement::isAtBusStop()
{
	if (parentPassenger->BoardedBus.get() == false)
		return true;
	else
		return false;
}

bool sim_mob::PassengerMovement::isDestBusStopReached()
{

	if (parentPassenger->AlightedBus.get() == true)//passenger alights when destination is reached
		return true;
	else
		return false;
}

Point2D sim_mob::PassengerMovement::getXYPosition()
{
	return Point2D(parentPassenger->parent->xPos.get(), parentPassenger->parent->yPos.get());
}

Point2D sim_mob::PassengerMovement::getDestPosition()
{
	return Point2D((destBusStop->xPos), (destBusStop->yPos));
}

bool sim_mob::PassengerMovement::PassengerAlightBus(Driver* driver)
{
	BusDriver* busdriver = dynamic_cast<BusDriver*> (driver);
	if (busdriver)
	{
		Bus* bus = dynamic_cast<Bus*> (busdriver->getVehicle());
		int xpos_approachingbusstop = busdriver->get_xPosApproachingBusStop();
		int ypos_approachingbusstop = busdriver->get_yPosApproachingBusStop();
		if (xpos_approachingbusstop - getDestPosition().getX() == 0 && ypos_approachingbusstop - getDestPosition().getY() == 0)
		{
			//alight-delete passenger agent from list
			bus->setPassengerCount(bus->getPassengerCount() - 1);
			parentPassenger->busdriver.set(nullptr); //driver would be null as passenger has alighted
			parentPassenger->AlightedBus.set(true);
			parentPassenger->BoardedBus.set(false);
			parentPassenger->parent->xPos.set(xpos_approachingbusstop);
			parentPassenger->parent->yPos.set(ypos_approachingbusstop);
			displayX = xpos_approachingbusstop;
			displayY = ypos_approachingbusstop;
			return true;
		}
	}
	else
	{
		parentPassenger->busdriver.set(nullptr); //passenger should store the bus driver
		parentPassenger->BoardedBus.set(false); //to indicate passenger has boarded bus
		parentPassenger->AlightedBus.set(true); //to indicate whether passenger has alighted bus
		parentPassenger->parent->xPos.set(driver->getCurrPosition().x);
		parentPassenger->parent->yPos.set(driver->getCurrPosition().y);
		displayX = driver->getCurrPosition().x;
		displayY = driver->getCurrPosition().y;
	}

	return false;
}

bool sim_mob::PassengerMovement::isBusBoarded()
{
	return (parentPassenger->BoardedBus.get() == true);
}

void sim_mob::PassengerMovement::findWaitingTime(Bus* bus)
{
	waitingTime = (bus->getTimeOfReachingBusStop())-(timeOfReachingBusStop);
}

BusStop* sim_mob::PassengerMovement::setBusStopXY(const Node* node)//to find the nearest busstop to a node
{
	const MultiNode* currEndNode = dynamic_cast<const MultiNode*> (node);
	double dist = 0;
	BusStop*bs1 = 0;
	if (currEndNode)
	{
		const std::set<sim_mob::RoadSegment*>& segments_ = currEndNode->getRoadSegments();
		BusStop* busStop_ptr = nullptr;
		for (std::set<sim_mob::RoadSegment*>::const_iterator i = segments_.begin(); i != segments_.end(); i++)
		{
			sim_mob::BusStop* bustop_ = (*i)->getBusStop();
			busStop_ptr = getBusStop(node, (*i));
			if (busStop_ptr)
			{
				double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos, node->location.getX(), node->location.getY());
				if ((newDist < dist || dist == 0) && busStop_ptr->BusLines.size() != 0)
				{
					dist = newDist;
					bs1 = busStop_ptr;
				}
			}
		}
	}
	else
	{
		Point2D point = node->location;
		const StreetDirectory::LaneAndIndexPair lane_index = StreetDirectory::instance().getLane(point);
		if (lane_index.lane_)
		{
			sim_mob::Link* link_ = lane_index.lane_->getRoadSegment()->getLink();
			const sim_mob::Link* link_2 = StreetDirectory::instance().searchLink(link_->getEnd(), link_->getStart());
			BusStop* busStop_ptr = nullptr;

			std::vector<sim_mob::RoadSegment*> segments_;

			if (link_)
			{
				segments_ = const_cast<Link*> (link_)->getSegments();
				for (std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin(); i != segments_.end(); i++)
				{
					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
					busStop_ptr = getBusStop(node, (*i));
					if (busStop_ptr)
					{
						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos, point.getX(), point.getY());
						if ((newDist < dist || dist == 0) && busStop_ptr->BusLines.size() != 0)
						{
							dist = newDist;
							bs1 = busStop_ptr;
						}
					}
				}
			}

			if (link_2)
			{
				segments_ = const_cast<Link*> (link_2)->getSegments();
				for (std::vector<sim_mob::RoadSegment*>::const_iterator i = segments_.begin(); i != segments_.end(); i++)
				{
					sim_mob::BusStop* bustop_ = (*i)->getBusStop();
					busStop_ptr = getBusStop(node, (*i));
					if (busStop_ptr)
					{
						double newDist = sim_mob::dist(busStop_ptr->xPos, busStop_ptr->yPos, point.getX(), point.getY());
						if ((newDist < dist || dist == 0) && busStop_ptr->BusLines.size() != 0)
						{
							dist = newDist;
							bs1 = busStop_ptr;
						}
					}
				}
			}
		}

	}

	return bs1;
}

bool sim_mob::PassengerMovement::PassengerBoardBus_Choice(Driver* driver)
{
	BusDriver* busdriver = dynamic_cast<BusDriver*> (driver);
	if (busdriver)
	{
		for (int i = 0; i < buslinesToTake.size(); i++)
		{
			Bus* bus = dynamic_cast<Bus*> (busdriver->getVehicle());
			if (buslinesToTake[i]->getBusLineID() == bus->getBusLineID())//boards if approaching busline of approaching busline
				//is in the pre-decided busline list
			{
				if (bus->getPassengerCount() + 1 <= bus->getBusCapacity())
				{
					bus->passengers_inside_bus.push_back(parentPassenger->parent);
					bus->setPassengerCount(bus->getPassengerCount() + 1);
					parentPassenger->busdriver.set(busdriver); //passenger should store the bus driver
					parentPassenger->BoardedBus.set(true); //to indicate passenger has boarded bus
					parentPassenger->AlightedBus.set(false); //to indicate whether passenger has alighted bus
					findWaitingTime(bus);
					buslinesToTake.clear();
					return true;
				}
			}
		}
	}
	else
	{
		parentPassenger->busdriver.set(busdriver); //passenger should store the bus driver
		parentPassenger->BoardedBus.set(true); //to indicate passenger has boarded bus
		parentPassenger->AlightedBus.set(false); //to indicate whether passenger has alighted bus
	}
	return false;
}

void sim_mob::PassengerMovement::FindBusLines() //find bus lines there and decide which line to board based on shortest path
{
	if (originBusStop != nullptr && parentPassenger->BoardedBus.get() == false)
	{
		vector<Busline*> buslines = originBusStop->BusLines; //list of available buslines at busstop
		int prev = 0;
		for (int i = 0; i < buslines.size(); i++)
		{
			/*query through the busstops for each available busline at the busstop
			and see if it goes to the passengers destination.If more than one busline avaiable
			choose the busline with the shortest path*/

			const std::vector<BusTrip>& BusTrips = buslines[i]->queryBusTrips();

			//busstops has the info about the list of bus stops a particular bus line goes to
			std::vector<const::BusStop*> busstops = BusTrips[0].getBusRouteInfo().getBusStops();

			std::vector<const::BusStop*>::iterator it;
			int noOfBusstops = 0;

			//queries through list of bus stops of bus line starting from current bus stop to end bus stop
			//this is to see if the particular bus line has the destination bus stop of passenger
			//if yes and if its shortest available route, the bus line is added to the list of bus lines passenger is supposed to take
			for (it = ++std::find(busstops.begin(), busstops.end(), originBusStop); it != busstops.end(); it++)
			{
				BusStop* bs = const_cast<BusStop*> (*it);

				//checking if the bus stop is the destination bus stop of passenger
				if (bs == destBusStop)//add this bus line to the list,if it is the shortest
				{
					if (prev == 0 || prev > noOfBusstops)
					{
						buslinesToTake.clear();
						buslinesToTake.push_back(buslines[i]);
						prev = noOfBusstops;
					}
					else if (prev == noOfBusstops)
					{
						buslinesToTake.push_back(buslines[i]); //update the list of bus lines passenger is supposed to take
						prev = noOfBusstops;
					}
				}
				noOfBusstops++;
			}
		}
	}
}

std::vector<Busline*> sim_mob::PassengerMovement::ReturnBusLines()
{
	return buslinesToTake;
}
}
