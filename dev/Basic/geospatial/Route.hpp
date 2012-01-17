#pragma once

#include "util/DynamicVector.hpp"
#include "BusStop.hpp"
#include "Lane.hpp"





namespace sim_mob{
	class Lane;
	class BusStop;

	/**
	 * \author Skyler Seto
	 * \author Seth N. Hetu
	 */
	class Route {
	public:
		
		Route(unsigned int id, std::vector <Lane*> lanes);
		
        int computeRouteLength(); //gives the length of the given route.  
        Lane* getFirstRoadLane(); //Initial start position of busroute
		Lane* getLastLane(); //final place bus goes.
		
		
	public:
		
		int id;
		std::vector <Lane*> lanes; 
		
		
		
	};
	

		
	
}








