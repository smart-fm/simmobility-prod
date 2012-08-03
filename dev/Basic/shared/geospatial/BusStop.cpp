/* Copyright Singapore-MIT Alliance for Research and Technology */


#include "Pavement.hpp"
#include "BusStop.hpp"
#include "RoadSegment.hpp"
#include "Lane.hpp"
#include "util/GeomHelpers.hpp"
#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#include "util/GeomHelpers.hpp"
#endif


using namespace sim_mob;
using std::vector;
using std::set;
using std::string;



double sim_mob::BusStop::EstimateStopPoint(double xPos, double yPos, const sim_mob::RoadSegment* rs)
{
	DynamicVector SegmentLength(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),rs->getStart()->location.getX(),rs->getStart()->location.getY());
	DynamicVector BusStopDistfromStart(xPos, yPos,rs->getStart()->location.getX(),rs->getStart()->location.getY());
	DynamicVector BusStopDistfromEnd(rs->getEnd()->location.getX(),rs->getEnd()->location.getY(),xPos,yPos);
	double a = BusStopDistfromStart.getMagnitude();
	double b = BusStopDistfromEnd.getMagnitude();
	double c = SegmentLength.getMagnitude();
	return (-b*b + a*a + c*c)/(2.0*c);
}
