/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <cmath>
#include <cassert>
#include <limits>

#include "Lane.hpp"
#include "../util/DynamicVector.hpp"
#include "../util/GeomHelpers.hpp"

using std::vector;

namespace sim_mob
{

namespace
{
    // Return the distance between the middle of the lane specified by <thisLane> and the middle
    // of the road-segment specified by <segment>; <thisLane> is one of the lanes in <segment>.
    double middle(const Lane& thisLane, const RoadSegment& segment)
    {
    	//If the segment width is unset, calculate it from the lane widths
        if (segment.width == 0) {
            for (vector<Lane*>::const_iterator it=segment.getLanes().begin(); it!=segment.getLanes().end(); it++) {
            	segment.width += (*it)->getWidth();
            }
        }

        //Retrieve half the segment width
        double w = segment.width / 2.0;
        if (w==0) {
        	//We can use default values here, but I've already hardcoded 300cm into too many places. ~Seth
        	throw std::runtime_error("Both the segment and all its lanes have a width of zero.");
        }
        w = 0; //Note: We should be incrementing, right? ~Seth

        //Maintain a default lane width
        double defaultLaneWidth = segment.width / segment.getLanes().size();

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

    // Return the point that is perpendicular (with magnitude <magnitude>) to the vector that begins at <origin> and
    // passes through <direction>. This point is left of the vector if <magnitude> is positive.
    Point2D getSidePoint(const Point2D& origin, const Point2D& direction, double magnitude) {
    	DynamicVector dv(origin.getX(), origin.getY(), direction.getX(), direction.getY());
    	dv.flipNormal(false).scaleVectTo(magnitude).translateVect();
    	return Point2D(dv.getX(), dv.getY());
    }


    // Return the intersection of the vectors (pPrev->pCurr) and (pNext->pCurr) when extended by "magnitude"
    Point2D calcCurveIntersection(const Point2D& pPrev, const Point2D& pCurr, const Point2D& pNext, double magnitude) {
    	//Get an estimate on the maximum distance. This isn't strictly needed, since we use the line-line intersection formula later.
    	double maxDist = sim_mob::dist(&pPrev, &pNext);

    	//Get vector 1.
    	DynamicVector dvPrev(pPrev.getX(), pPrev.getY(), pCurr.getX(), pCurr.getY());
    	dvPrev.translateVect().flipNormal(false).scaleVectTo(magnitude).translateVect();
    	dvPrev.flipNormal(true).scaleVectTo(maxDist);

    	//Get vector 2
    	DynamicVector dvNext(pNext.getX(), pNext.getY(), pCurr.getX(), pCurr.getY());
    	dvNext.translateVect().flipNormal(true).scaleVectTo(magnitude).translateVect();
    	dvNext.flipNormal(false).scaleVectTo(maxDist);

    	//Compute their intersection. We use the line-line intersection formula because the vectors
    	// won't intersect for acute angles.
    	return LineLineIntersect(dvPrev, dvNext);
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

	//Sanity check.
	if (polyline_.size()<2) {
		throw std::runtime_error("Can't extend a polyline of size 0 or 1.");
	}

	//Push back the first point
	// We assume that the lanes at the start and end points of the road segments
	// are "aligned", that is, first and last point in the lane's polyline are
	// perpendicular to the road-segment polyline at the start and end points.
	const vector<Point2D>& poly = parentSegment_->polyline;
	polyline_.push_back(getSidePoint(poly.at(0), poly.at(1), distToMidline));

	//Iterate through pairs of points in the polyline.
	for (size_t i=1; i<poly.size()-1; i++) {
		//If the road segment pivots, we need to extend the relevant vectors and find their intersection.
		// That is the point which we intend to add.
		Point2D p = calcCurveIntersection(poly[i-1], poly[i], poly[i+1], distToMidline);
		if (p.getX()==std::numeric_limits<double>::max()) {
			//The lines are parallel; just extend them like normal.
			p = getSidePoint(poly[i], poly[i+1], distToMidline);
		}

		polyline_.push_back(p);
	}

	//Push back the last point
	//NOTE: Check that this is correct; negating the distance should work just fine with our algorithm.
	polyline_.push_back(getSidePoint(poly.at(poly.size()-1), poly.at(poly.size()-2), -distToMidline));
}



const std::vector<sim_mob::Point2D>& Lane::getPolyline() const
{
    //Recompute the polyline if needed
    if (polyline_.empty()) {
    	parentSegment_->syncLanePolylines();
    }
    return polyline_;
}

}
