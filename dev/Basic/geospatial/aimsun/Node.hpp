#pragma once

#include "Base.hpp"

namespace sim_mob
{
namespace aimsun
{

///An AIMSUN road intersection or segment intersection.
class Node : public Base {
public:
	double xPos;
	double yPos;
	bool isIntersection;
};


}
}
