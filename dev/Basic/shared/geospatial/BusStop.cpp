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


// this was supposed to be a function to return the left most lane of a segment but since that can directly be accessed
// as in 'Point A' so it has no further functionality
//TODO: This function may be buggy. ~Seth
/*int sim_mob::BusStop::bus_stop_lane(const RoadSegment& segment)
   {
        if (segment.width==0) {
       	throw std::runtime_error("Both the segment and all its lanes have a width of zero.");
       }
        else
        {
        	unsigned int laneID = segment.getLanes().back()->getLaneID(); // Point A
        	const std::vector<sim_mob::Point2D>& res = lane_location->getRoadSegment()->getLaneEdgePolyline(laneID);
        	return res.size();
        }
       }

}*/

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
