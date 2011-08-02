/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "Base.hpp"

namespace sim_mob
{
namespace aimsun
{

//Forward declarations
class Section;


///An AIMSUN road intersection or segment intersection.
class Node : public Base {
public:
	double xPos;
	double yPos;
	bool isIntersection;

	//Decorated data
	std::vector<Section*> sectionsAtNode;
	bool candidateForSegmentNode;

};


}
}
