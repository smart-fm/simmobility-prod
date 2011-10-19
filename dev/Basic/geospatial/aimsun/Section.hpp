#pragma once

#include <string>

#include "Base.hpp"
#include "geospatial/Point2D.hpp"
#include "constants.h"

namespace sim_mob
{

//Forward declarations
class RoadSegment;

namespace aimsun
{

//Forward declarations
class Lane;
class Node;
class Turning;
class Polyline;

///An AIMSUN link or road segment
class Section : public Base {
public:
	std::string roadName;
	int numLanes;
	double speed;
	double capacity;
	double length;
	Node* fromNode;
	Node* toNode;

	Section() : Base(), fromNode(nullptr), toNode(nullptr), generatedSegment(nullptr) {}

	//Placeholders
	int TMP_FromNodeID;
	int TMP_ToNodeID;

	//Decorated data
	std::vector<Turning*> connectedTurnings;
	std::vector<Polyline*> polylineEntries;
	std::map<int, std::vector<Lane*> > laneLinesAtNode; //Arranged by laneID
	std::vector< std::vector<sim_mob::Point2D> > lanePolylinesForGenNode;

	//Reference to saved object
	sim_mob::RoadSegment* generatedSegment;
};


}
}
