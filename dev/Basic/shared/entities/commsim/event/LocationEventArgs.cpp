//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "LocationEventArgs.hpp"

#include "entities/commsim/serialization/CommsimSerializer.hpp"


using namespace sim_mob;

sim_mob::LocationEventArgs::LocationEventArgs(const sim_mob::Agent* agent) :agent(agent)
{
}

sim_mob::LocationEventArgs::~LocationEventArgs()
{
}

std::string sim_mob::LocationEventArgs::serialize() const
{
	//Attempt to reverse-project the Agent's (x,y) location into Lat/Lng, if such a projection is possible.
	LatLngLocation loc;
	CoordinateTransform* trans = ConfigManager::GetInstance().FullConfig().getNetwork().getCoordTransform(false);
	if (trans) {
		loc = trans->transform(DPoint(agent->xPos.get(), agent->yPos.get()));
	}

	return CommsimSerializer::makeLocation(agent->xPos.get(), agent->yPos.get(), loc);
}

/*
Json::Value sim_mob::LocationEventArgs::toJSON() const
{
	//Attempt to reverse-project the Agent's (x,y) location into Lat/Lng, if such a projection is possible.
	LatLngLocation loc;
	CoordinateTransform* trans = ConfigManager::GetInstance().FullConfig().getNetwork().getCoordTransform(false);
	if (trans) {
		loc = trans->transform(DPoint(agent->xPos.get(), agent->yPos.get()));
	}

	return JsonParser::makeLocationMessage(agent->xPos.get(), agent->yPos.get(), loc);
}
 */
