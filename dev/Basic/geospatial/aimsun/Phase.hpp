/* Copyright Singapore-MIT Alliance for Research and Technology */

//vahid
#pragma once
namespace sim_mob { namespace aimsun {
class Phase {
public:
	Section* ToSection;
	int sectionTo;
	Section* FromSection;
	int sectionFrom;
	int nodeId;
	std::string name;
	int laneFrom_A;
	int laneFrom_B;
	int laneTo_A;
	int laneTo_B;
};
}}
