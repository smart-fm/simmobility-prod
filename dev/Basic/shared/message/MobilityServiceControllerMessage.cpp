/*
 * MobilityServiceControllerMessage.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha, Andrea Araldo
 */

#include "MobilityServiceControllerMessage.hpp"
#include "geospatial/network/RoadNetwork.hpp"

namespace sim_mob
{

const Schedule &SchedulePropositionMessage::getSchedule() const
{
	return schedule;
};

bool TripRequestMessage::operator==(const TripRequestMessage &other) const
{
	if ( timeOfRequest == other.timeOfRequest &&
			userId == other.userId &&
			startNodeId == other.startNodeId &&
			destinationNodeId == other.destinationNodeId &&
			extraTripTimeThreshold == other.extraTripTimeThreshold)
	{
		return true;
	}
	else
	{
		return false;
	}
};


bool TripRequestMessage::operator!=(const TripRequestMessage &other) const
{
	if (timeOfRequest != other.timeOfRequest ||
			userId != other.userId ||
			startNodeId != other.startNodeId ||
			destinationNodeId != other.destinationNodeId ||
			extraTripTimeThreshold != other.extraTripTimeThreshold)
	{
		return true;
	}
	else
	{
		return false;
	}
};

bool TripRequestMessage::operator>(const TripRequestMessage &other) const
{
	if (operator==(other) || operator<(other))
	{
		return false;
	}
	return true;
}

bool TripRequestMessage::operator<(const TripRequestMessage &other) const
{
	if (timeOfRequest < other.timeOfRequest)
	{
		return true;
	}

	if (timeOfRequest == other.timeOfRequest)
	{
		// The order here is arbitrary
		if (userId < other.userId)
		{
			return true;
		}

#ifndef NDEBUG
		if (userId == other.userId && !operator==(other) )
		{
			std::stringstream msg; msg <<"There exist two requests from the same user "<< userId<<
				" issued in the same frame "<< timeOfRequest.frame() << ". Is this an error?";
			throw std::runtime_error(msg.str() );
		}
#endif
	}
	return false;
}

bool ScheduleItem::operator<(const ScheduleItem &other) const
{
/*
#ifndef NDEBUG
	if ((scheduleItemType != DROPOFF && scheduleItemType != PICKUP) ||
			(other.scheduleItemType != DROPOFF && other.scheduleItemType != PICKUP))
	{
		throw std::runtime_error("You can only compare pickups and drop-offs. If you are comparing other types of schedule items, it may be an error ");
	}
#endif
*/

	if (tripRequest < other.tripRequest)
	{
		return true;
	}
	if (tripRequest == other.tripRequest && scheduleItemType == PICKUP && other.scheduleItemType == DROPOFF)
	{
		return true;
	}

	return false;
}

bool ScheduleItem::operator==(const ScheduleItem &rhs) const
{
	bool result = false;

	switch (scheduleItemType)
	{
	case PICKUP:
	case DROPOFF:
	{
		//If they are the same type, compare the related data
		if (scheduleItemType == rhs.scheduleItemType)
		{
			result = (tripRequest == rhs.tripRequest);
		}

		break;
	}
	case CRUISE:
	{
		if(rhs.scheduleItemType == CRUISE)
		{
			result = (nodeToCruiseTo == rhs.nodeToCruiseTo);
		}

		break;
	}
	case PARK:
	{
		if(rhs.scheduleItemType == PARK)
		{
			result = (parking == parking);
		}

		break;
	}
	}

	return result;
}

//{ SCHEDULE FUNCTIONS
const std::vector<ScheduleItem>& Schedule::getItems() const
{	return items; }

const size_t Schedule::size() const{ return items.size(); }

const bool Schedule::empty() const{ return items.empty(); }

Schedule::iterator Schedule::begin() {return items.begin(); };
Schedule::const_iterator Schedule::begin() const {return items.begin(); };
Schedule::const_iterator Schedule::end() const {return items.end(); };
Schedule::iterator Schedule::end(){return items.end(); };

void Schedule::insert(std::vector<ScheduleItem>::iterator position, const ScheduleItem scheduleItem)
{
	items.insert(position, scheduleItem);
	onAddingScheduleItem(scheduleItem);
};

void Schedule::insert(Schedule::iterator position, Schedule::iterator first, Schedule::iterator last)
{
	items.insert(position, first, last);
	for (const_iterator it = first ; it != last; ++it)
		onAddingScheduleItem(*it);
};


const ScheduleItem& Schedule::back() const
{	return items.back(); }

const ScheduleItem& Schedule::front() const
{	return items.front(); }

ScheduleItem& Schedule::front()
{	return items.front(); }

void Schedule::pop_back()
{
	onRemovingScheduleItem(back() );
	items.pop_back();
}
void Schedule::push_back(ScheduleItem item)
{
	onAddingScheduleItem(item);
	items.push_back(item);
};

Schedule::iterator Schedule::erase(Schedule::iterator position)
{
	onRemovingScheduleItem(*position);
	return items.erase(position);
}

const ScheduleItem& Schedule::at(size_t n) const
{	return items.at(n); }

ScheduleItem& Schedule::at(size_t n)
{	return items.at(n); }

void Schedule::onAddingScheduleItem(const ScheduleItem& item)
{
	switch (item.scheduleItemType)
	{
		case DROPOFF:
		{
			//aa!!: Once the destinationNode pointer, instead of the destinationNodeId, will be available, I will not need this overhead
			const RoadNetwork *rdNetowrk = RoadNetwork::getInstance();
			const Node *dropOffNode = rdNetowrk->getById(rdNetowrk->getMapOfIdvsNodes(), item.tripRequest.destinationNodeId );
			if (doWeComputeBarycenter)
			{
				dropOffBarycenter.setX( ( dropOffBarycenter.getX() * passengerCount + dropOffNode->getLocation().getX() ) / (passengerCount+1) );
				dropOffBarycenter.setY( ( dropOffBarycenter.getY() * passengerCount + dropOffNode->getLocation().getY() ) / (passengerCount+1) );
			}
			passengerCount++;
			break;
		}
		// otherwise, do nothing
	}
}

void Schedule::onRemovingScheduleItem(const ScheduleItem& item)
{
	switch (item.scheduleItemType)
	{
		case DROPOFF:
		{
			//aa!!: Once the destinationNode pointer, instead of the destinationNodeId, will be available, I will not need this overhead
			const RoadNetwork *rdNetowrk = RoadNetwork::getInstance();
			const Node *dropOffNode = rdNetowrk->getById(rdNetowrk->getMapOfIdvsNodes(), item.tripRequest.destinationNodeId );
			if (doWeComputeBarycenter)
			{
				dropOffBarycenter.setX( ( dropOffBarycenter.getX() * passengerCount - dropOffNode->getLocation().getX() ) / (passengerCount+1) );
				dropOffBarycenter.setY( ( dropOffBarycenter.getY() * passengerCount - dropOffNode->getLocation().getY() ) / (passengerCount+1) );
			}
			passengerCount--;
			break;
		}
		// otherwise, do nothing
	}
}

const Point& Schedule::getDropOffBarycenter() const
{ return dropOffBarycenter;}

short Schedule::getPassengerCount() const {	return passengerCount; }

//} SCHEDULE FUNCTIONS





} // End of namespace


std::ostream& operator<<(std::ostream& strm, const sim_mob::TripRequestMessage& request)
{
	return strm << "request issued by "<<request.userId<< " at " << request.timeOfRequest<<
			" to go from node "<< request.startNodeId <<
			", to node "<< request.destinationNodeId;
}

std::ostream& operator<<(std::ostream& strm, const sim_mob::ScheduleItem& item)
{
	strm << "ScheduleItem ";
	switch (item.scheduleItemType)
	{
		case (sim_mob::ScheduleItemType::PICKUP):
		{
			strm<<"PICKUP of "<< item.tripRequest;
			break;
		}
		case (sim_mob::ScheduleItemType::DROPOFF):
		{
			strm<<"DROPOFF of "<<item.tripRequest;
			break;
		}
		case (sim_mob::ScheduleItemType::CRUISE):
		{
			const sim_mob::Node* nodeToCruiseTo = item.nodeToCruiseTo;
#ifndef NDEBUG
			if (!nodeToCruiseTo)
				throw std::runtime_error("Invalid CRUISE ScheduleItem: the nodeToCruiseTo is NULL");
#endif
			strm<<"CRUISE to node "<<item.nodeToCruiseTo->getNodeId();
			break;
		}
		case (sim_mob::ScheduleItemType::PARK):
		{
			strm<<"Parking to "<<item.parking->getParkingId();
			break;
		}
		default:{throw std::runtime_error("unrecognized schedule item type");}
	};
	return strm;
}

std::ostream& operator<<(std::ostream& strm, const sim_mob::Schedule& schedule)
{
	strm<<"Schedule [";
	for (const sim_mob::ScheduleItem& item : schedule)
	{
		strm<< item <<", ";
	}
	strm<<" ]";
	return strm;
}

bool operator==(sim_mob::TripRequestMessage r1, const sim_mob::TripRequestMessage r2)
{
	return r1.operator ==(r2);
}
