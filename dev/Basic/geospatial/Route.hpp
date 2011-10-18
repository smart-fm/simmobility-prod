#pragma once


#include "../util/DynamicVector.hpp"


namespace sim_mob{
	class RoadSegment;
	class BusStop;

	class Route{
	public:

		Route(int id, std::vector <RoadSegment*> roadsegments, int routetime);

        int computeRouteLength(); ///<gives the length of the given route.
        const RoadSegment* getCurrentRoadSegment() const; ///<currently where bus is located on route
        const RoadSegment* getNextRoadSegment() const;    ///<where/ which direction the bus is going
        const RoadSegment* getFirstRoadSegment() const;  ///<Initial start position of busroute
        const RoadSegment* getLastRoadSegment() const;   ///<final place bus goes.


	//public:
	protected:

		int id;
		//DynamicVector <RoadSegment*> roadsegments; //NOTE: DynamicVector is for mathematical vectors. It's not a container.
		int routetime;



	};

}







