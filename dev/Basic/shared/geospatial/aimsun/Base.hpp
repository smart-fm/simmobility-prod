/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

namespace sim_mob
{
namespace aimsun
{

///Nearly all AIMSUN items have IDs.
/// \author Seth N. Hetu
class Base {
public:
	int id;

	Base() : id(0), hasBeenSaved(false) {}

	//Write flag
	bool hasBeenSaved;
};


}
}
