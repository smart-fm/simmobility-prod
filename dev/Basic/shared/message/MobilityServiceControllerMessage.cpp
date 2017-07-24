/*
 * MobilityServiceControllerMessage.cpp
 *
 *  Created on: Apr 18, 2017
 *      Author: Akshay Padmanabha, Andrea Araldo
 */

#include "MobilityServiceControllerMessage.hpp"

namespace sim_mob
{
	const Schedule& SchedulePropositionMessage::getSchedule() const
	{
		return schedule;
	};

	bool TripRequestMessage::operator==(const TripRequestMessage& other) const
	{
		if (
			timeOfRequest == other.timeOfRequest &&
			userId == other.userId &&
			startNodeId == other.startNodeId &&
			destinationNodeId == other.destinationNodeId &&
			extraTripTimeThreshold == other.extraTripTimeThreshold
		) {return true; }
		else  {return false;}
	};


	bool TripRequestMessage::operator!=(const TripRequestMessage& other) const
	{
		if (
			timeOfRequest != other.timeOfRequest ||
			userId != other.userId ||
			startNodeId != other.startNodeId ||
			destinationNodeId != other.destinationNodeId ||
			extraTripTimeThreshold != other.extraTripTimeThreshold
		) {return true; }
		else  {return false;}
	};

	bool TripRequestMessage::operator>(const TripRequestMessage& other) const
	{
		if ( operator==(other) || operator<(other) )
			return false;
		return true;
	}

	bool TripRequestMessage::operator<(const TripRequestMessage& other) const
	{
		if (timeOfRequest < other.timeOfRequest ) return true;
		if (timeOfRequest == other.timeOfRequest)
		{
			// The order here is arbitrary
			if (userId < other.userId) return true;

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

	bool ScheduleItem::operator <(const ScheduleItem& other) const
	{
		/*
#ifndef NDEBUG
		if (	(scheduleItemType!=DROPOFF && scheduleItemType!=PICKUP)
			||	(other.scheduleItemType!=DROPOFF && other.scheduleItemType!=PICKUP)
			)
			throw std::runtime_error("You can only compare pickups and dropoofs. If you are comparing other types of schedule items, it may be an error ");
#endif
		 */
		if ( tripRequest < other.tripRequest ) return true;
		if ( tripRequest == other.tripRequest && scheduleItemType==PICKUP && other.scheduleItemType==DROPOFF) return true;
		return false;
	}
}



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
			strm<<"Parking to "<<item.parkingId;
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
