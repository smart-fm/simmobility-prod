#pragma once

#include "util/DynamicVector.hpp"

#include "Route.hpp"
#include <vector>




namespace sim_mob{
	class RoadSegment;
	class BusStop;

	class BusRoute: public Route
	{
	public:
		BusRoute(int id, std::vector <RoadSegment*> roadsegments, int routetime, std::vector <BusStop*> busstops) :
		Route(id, roadsegments, routetime) {}


	};

}







