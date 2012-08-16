/* Copyright Singapore-MIT Alliance for Research and Technology */

#pragma once

#include <vector>

#include "RoadItem.hpp"
#include "Point2D.hpp"


namespace sim_mob
{
#ifndef SIMMOB_DISABLE_MPI
class PackageUtils;
class UnPackageUtils;
#endif

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
	Crossing() : RoadItem() {}


//protected:
	//The line (start/end points that make up the line) "near" the intersection
	std::pair<sim_mob::Point2D, sim_mob::Point2D> nearLine;

	//The line that is "far" from the intersection (further down the road)
	std::pair<sim_mob::Point2D, sim_mob::Point2D> farLine;
	unsigned int crossingID;

public:
	void setCrossingID(unsigned int crossingID_){crossingID = crossingID_;}
	unsigned int getCrossingID(){return crossingID;}
#ifndef SIMMOB_DISABLE_MPI
	///The identification of Crossing is packed using PackageUtils;
	static void pack(PackageUtils& package, Crossing* one_cross);

	///UnPackageUtils use the identification of Crossing to find the Crossing Object
	static const Crossing* unpack(UnPackageUtils& unpackage);
#endif
};





}
