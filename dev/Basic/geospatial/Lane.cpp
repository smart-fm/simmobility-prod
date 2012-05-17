/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <cmath>
#include <cassert>
#include <limits>

#include "Lane.hpp"
#include "util/DynamicVector.hpp"
#include "util/GeomHelpers.hpp"

#ifndef SIMMOB_DISABLE_MPI
#include "partitions/PackageUtils.hpp"
#include "partitions/UnPackageUtils.hpp"
#endif

using std::vector;

namespace sim_mob
{

namespace
{
    // Return the distance between the middle of the lane specified by <thisLane> and the middle
    // of the road-segment specified by <segment>; <thisLane> is one of the lanes in <segment>.
    double middle(const Lane& thisLane, const RoadSegment& segment)
    {
        //Retrieve half the segment width
        //double w = segment.width / 2.0;
        if (segment.width==0) {
        	//We can use default values here, but I've already hardcoded 300cm into too many places. ~Seth
        	throw std::runtime_error("Both the segment and all its lanes have a width of zero.");
        }
        double w = 0; //Note: We should be incrementing, right? ~Seth

        //Maintain a default lane width
        double defaultLaneWidth = static_cast<double>(segment.width) / segment.getLanes().size();

        //Iterate through each lane, reducing the return value by each lane's width until you reach the current lane.
        // At that point, reduce by half the lane's width and return.
        for (vector<Lane*>::const_iterator it=segment.getLanes().begin(); it!=segment.getLanes().end(); it++) {
        	double thisLaneWidth = (*it)->getWidth()>0 ? (*it)->getWidth() : defaultLaneWidth;
            if (*it != &thisLane) {
                w += thisLaneWidth;
            } else {
                w += (thisLaneWidth / 2.0);
                return w;
            }
        }

        //Exceptions don't require a fake return.
        //But we still need to figure out whether we're using assert() or exceptions for errors! ~Seth
        throw std::runtime_error("middle() called on a Lane not in this Segment.");
    }


}  //End anonymous namespace.



//This contains most of the functionality from getPolyline(). It is called if there
// is no way to determine the polyline from the lane edges (i.e., they don't exist).
void Lane::makePolylineFromParentSegment()
{
	double distToMidline = middle(*this, *parentSegment_);
	if (distToMidline==0) {
		throw std::runtime_error("No side point for line with zero magnitude.");
	}

	//Set the width if it hasn't been set
	if (width_==0) {
		width_ = parentSegment_->width/parentSegment_->getLanes().size();
	}

	//Save
	const vector<Point2D>& poly = parentSegment_->polyline;
	polyline_ = sim_mob::ShiftPolyline(poly, distToMidline);
}



const std::vector<sim_mob::Point2D>& Lane::getPolyline() const
{
    //Recompute the polyline if needed
    if (polyline_.empty()) {
    	parentSegment_->syncLanePolylines();
    }
    return polyline_;
}

#ifndef SIMMOB_DISABLE_MPI

#endif

}
