#pragma once

#include <string>

#include "Base.hpp"
#include "../../constants.h"

namespace sim_mob
{
namespace aimsun
{

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

	Section() : fromNode(nullptr), toNode(nullptr) {}

	//Placeholders
	int TMP_FromNodeID;
	int TMP_ToNodeID;
};


}
}
