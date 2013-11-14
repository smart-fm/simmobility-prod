//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "defaults.hpp"

#include<string>
#include<sstream>
#include<iostream>

#include "geospatial/Link.hpp"
#include "geospatial/Crossing.hpp"

namespace sim_mob {

sim_mob::LinkAndCrossing::LinkAndCrossing(int id_,sim_mob::Link const * link_,sim_mob::Crossing const * crossing_,double angle_):
			id(id_),
			link(link_),
			crossing(crossing_),
			angle(angle_)
{}
sim_mob::LinkAndCrossing::LinkAndCrossing(){
	id = -1;
	link = 0;
	crossing = 0;
	angle = -1;
}
size_t sim_mob::LinkAndCrossing::getId() const{
	return id;
}

bool sim_mob::LinkAndCrossingComparison::operator() (const LinkAndCrossing&a, const LinkAndCrossing&b){
		return a.link > b.link;
	}
}



