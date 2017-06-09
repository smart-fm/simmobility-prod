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
			currTick == other.currTick &&
			personId == other.personId &&
			startNodeId == other.startNodeId &&
			destinationNodeId == other.destinationNodeId &&
			extraTripTimeThreshold == other.extraTripTimeThreshold
		) {return true; }
		else  {return false;}
	};


	bool TripRequestMessage::operator!=(const TripRequestMessage& other) const
	{
		if (
			currTick != other.currTick ||
			personId != other.personId ||
			startNodeId != other.startNodeId ||
			destinationNodeId != other.destinationNodeId ||
			extraTripTimeThreshold != other.extraTripTimeThreshold
		) {return true; }
		else  {return false;}
	};
}



