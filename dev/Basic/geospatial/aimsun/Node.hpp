#pragma once

#include "Base.hpp"

namespace sim_mob
{
namespace aimsun
{

///An AIMSUN road intersection or segment intersection.
class Node : public Base {
	Node(unsigned int id) : Base(id) {}

	double xPos;
	double yPos;
	bool isIntersection;
};


}
}
