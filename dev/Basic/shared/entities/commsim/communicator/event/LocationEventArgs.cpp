//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * LocationEventArgs.cpp
 *
 *  Created on: May 28, 2013
 *      Author: vahid
 */

#include "LocationEventArgs.hpp"

#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "entities/Agent.hpp"
#include "geospatial/coord/CoordinateTransform.hpp"
#include "util/GeomHelpers.hpp"

namespace sim_mob {

LocationEventArgs::LocationEventArgs(const sim_mob::Agent * agent_) :agent(agent_){
	// TODO Auto-generated constructor stub

}
//std::string LocationEventArgs::ToJSON()const{
//	return JsonParser::makeLocationData(agent->xPos.get(), agent->yPos.get());
//}
Json::Value LocationEventArgs::ToJSON()const
{
	//Convert the agent's position (x,y) to (lat,lng) before serializing it.
	sim_mob::LatLngLocation pos = ConfigManager::GetInstance().FullConfig().getNetwork().getCoordTransform()->transform(DPoint(agent->xPos.get(), agent->yPos.get()));
	return JsonParser::makeLocationData(pos.latitude, pos.longitude);
}

LocationEventArgs::~LocationEventArgs()
{}

} /* namespace sim_mob */
