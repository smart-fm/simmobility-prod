//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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
