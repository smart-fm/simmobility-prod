//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
// license.txt (http://opensource.org/licenses/MIT)

#pragma once

#include <stdexcept>

#include "geospatial/network/Lane.hpp"
#include "geospatial/network/RoadSegment.hpp"

using namespace sim_mob;
using namespace std;

class no_turning_path_exception : public runtime_error
{
public:
	const Lane *fromLane;
	const RoadSegment *toSegment;
	
	explicit no_turning_path_exception(const string &arg, const Lane *fLane, const RoadSegment *tSegment) : runtime_error(arg), fromLane(fLane), toSegment(tSegment)
	{		
	}
};
