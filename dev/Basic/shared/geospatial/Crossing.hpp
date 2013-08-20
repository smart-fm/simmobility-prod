//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <vector>
#include <stdexcept>

#include "conf/settings/DisableMPI.h"

#include "geospatial/RoadItem.hpp"
#include "geospatial/Point2D.hpp"

namespace geo {
class crossing_t_pimpl;
} //End namespace geo

namespace sim_mob
{
class RoadSegment;
class PackageUtils;
class UnPackageUtils;

/**
 * A Crossing is a RoadItem that crosses one (or more) RoadSegment(s).
 * \author Seth N. Hetu
 *
 * \note
 * Currently, the "start" and "end" points of a Crossing don't make much sense; the crossing
 * already has a "far" and "near" line.
 */
class Crossing : public RoadItem {
public:
	Crossing() : RoadItem() /*crossingID(0),*/ {}

//	RoadSegment* getRoadSegment() const;
//	void setRoadSegment(RoadSegment *rs);

	//protected:
	//The line (start/end points that make up the line) "near" the intersection
	std::pair<sim_mob::Point2D, sim_mob::Point2D> nearLine;

	//The line that is "far" from the intersection (further down the road)
	std::pair<sim_mob::Point2D, sim_mob::Point2D> farLine;
//	unsigned int crossingID;

public:
	const unsigned int getCrossingID() const{return  getRoadItemID();}


public:
	void setCrossingID(unsigned int crossingID_){setRoadItemID(crossingID_); }
#ifndef SIMMOB_DISABLE_MPI
	///The identification of Crossing is packed using PackageUtils;
	static void pack(PackageUtils& package, Crossing* one_cross);

	///UnPackageUtils use the identification of Crossing to find the Crossing Object
	static const Crossing* unpack(UnPackageUtils& unpackage);
#endif
};





}
