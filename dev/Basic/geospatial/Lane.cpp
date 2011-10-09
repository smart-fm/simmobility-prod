/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <math.h>
#include <assert.h>

#include "Lane.hpp"
#include "../util/DynamicVector.hpp"

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

        //assert(false); // We shouldn't reach here.
        //return w;
    }

    // Return the point that is perpendicular to the line that passes through <p> and 
    // is sloping <dx> horizontally and <dy> vertically.  The distance between <p> and the
    // returned point is <w>.  If <w> is negative, the returned point is "above" the line;
    // otherwise it is below the line.
    Point2D getSidePoint(const Point2D& p, int dx, int dy, double w) {
    	//NOTE: The original equation returned values that were slightly off (re-enable to see).
    	//      I'm replacing this with a simple vector computation. ~Seth
    	if (w==0) {
    		throw std::runtime_error("No side point for line with zero width.");
    	}
    	DynamicVector dv(p.getX(), p.getY(), p.getX()+dx, p.getY()+dy);
    	dv.flipNormal(w<0).scaleVectTo(w).translateVect();
    	return Point2D(dv.getX(), dv.getY());
    }


    /*Point2D getSidePoint(const Point2D& p, int dx, int dy, double w)
    {
        // The parameterized equation of the line L is given by
        //     X = x + t * dx
        //     Y = y + t * dy
        // The line passes through p = (x, y) at t = 0.
        //
        // The parameterized equation for the line perpendicular to L is
        //     X = x + t * -dy
        //     Y = y + t * dx
        // This perpendicular line is above L for t > 0, and below L for t < 0.
        // Solving for t in terms of X, we have
        //     t = (X - x) / -dy
        // Substituting this into the Y equation, we get
        //     Y = y + (X - x) * dx / -dy
        //       = m*X + c
        // where m = dx / -dy and c = y - m*x
        //
        // Since the distance between (X, Y) and p is w, we have
        //     (X-x)*(X-x) + (Y-y)*(Y-y) = w*w
        //     (X-x)*(X-x) + (m*X + c-y)*(m*X + c-y) = w*w
        //     (X-x)*(X-x) + (m*X - m*x)*(m*X - m*x) = w*w
        //     (X-x)*(X-x) + m(X-x)m(X-x) = w*w
        //     (m*m + 1)(X -x)^2 = w^2
        // Solving, we have
        //     X = x + w / sqrt(m*m + 1)

        double m = static_cast<double>(dx) / -dy;
        if (w > 0)
        {
            if (dy > 0)
            {
                double x = p.getX() + w / sqrt(m*m + 1);
                double y = m*x + p.getY() - m*p.getX();
                return Point2D(x, y);
            }
            else
            {
                double x = p.getX() - w / sqrt(m*m + 1);
                double y = m*x + p.getY() - m*p.getX();
                return Point2D(x, y);
            }
        }
        else
        {
            if (dy > 0)
            {
                double x = p.getX() - w / sqrt(m*m + 1);
                double y = m*x + p.getY() - m*p.getX();
                return Point2D(x, y);
            }
            else
            {
                double x = p.getX() + w / sqrt(m*m + 1);
                double y = m*x + p.getY() - m*p.getX();
                return Point2D(x, y);
            }
        }
    }*/

    // Return the intersection of the line passing through <p1> and sloping <dx1> horizontally
    // and <dy1> vertically with the line passing through <p2> and sloping <dx2> horizontally
    // and <dy2> vertically.
    Point2D
    intersection(const Point2D& p1, int dx1, int dy1, const Point2D& p2, int dx2, int dy2)
    {
        // The parameterized equations for the two lines are
        //     x = x1 + t1 * dx1
        //     y = y1 + t1 * dy1
        // and
        //     x = x2 + t2 * dx2
        //     y = y2 + t2 * dy2
        //
        // At the intersection point, we have
        //     x1 + t1 * dx1 = x2 + t2 * dx2
        //     y1 + t1 * dy1 = y2 + t2 * dy2
        // Re-arranging
        //     dx1*t1 - dx2*t2 = x2-x1
        //     dy1*t1 - dy2*t2 = y2-y1
        // In matrix form, we have
        //     (dx1  -dx2) (t1) = (x2-x1)
        //     (dy1  -dy2) (t2) = (y2-y1)
        // Solving, we have
        //     (t1) = (-dy2  dx2) (x2-x1) / (-dx1*dy2 + dx2*dy1)
        //     (t2) = (-dy1  dx1) (y2-y1) / (-dx1*dy2 + dx2*dy1)
        // Therefore
        //     t1 = (-dy2*(x2-x1) + dx2*(y2-y1)) / (-dx1*dy2 + dx2*dy1)

        int x1 = p1.getX();
        int y1 = p1.getY();
        int x2 = p2.getX();
        int y2 = p2.getY();
        double t = static_cast<double>(-dy2) * (x2 - x1);
        t += static_cast<double>(dx2) * (y2 - y1);
        t /= (static_cast<double>(-dx1) * dy2 + static_cast<double>(dx2) * dy1);

        int x = x1 + t * dx1;
        int y = y1 + t * dy1;
        return Point2D(x, y);
    }
}



//This contains most of the functionality from getPolyline(). It is called if there
// is no way to determine the polyline from the lane edges (i.e., they don't exist).
void Lane::makePolylineFromParentSegment()
{
	double distToMidline = middle(*this, *parentSegment_);

	//Set the width if it hasn't been set
	if (width_==0) {
		width_ = parentSegment_->width/parentSegment_->getLanes().size();
	}

	//Iterate through pairs of points in the polyline.
	for (size_t i=0; i<parentSegment_->polyline.size()-1; i++)
	{
		const Point2D& p1 = parentSegment_->polyline[i];
		const Point2D& p2 = parentSegment_->polyline[i + 1];
		if (0 == i)
		{
			// We assume that the lanes at the start and end points of the road segments
			// are "aligned", that is, first and last point in the lane's polyline are
			// perpendicular to the road-segment polyline at the start and end points.
			int dx = p2.getX() - p1.getX();
			int dy = p2.getY() - p1.getY();
			Point2D p = getSidePoint(p1, dx, dy, distToMidline);
			polyline_.push_back(p);
		}
		else
		{
			const Point2D& p0 = parentSegment_->polyline[i - 1];
			int dx1 = p1.getX() - p0.getX();
			int dy1 = p1.getY() - p0.getY();
			int dx2 = p2.getX() - p1.getX();
			int dy2 = p2.getY() - p1.getY();

			// p0, p1, and p2 are in the middle of the road segment, not in the lane.
			// point1 and point2 below are in the middle of the lane.  So will be <p>.
			Point2D point1 = getSidePoint(p0, dx1, dy1, distToMidline);
			Point2D point2 = getSidePoint(p2, dx2, dy2, distToMidline);
			Point2D p = intersection(point1, dx1, dy1, point2, dx2, dy2);
			polyline_.push_back(p);

			// If this is the last line, then we add the end point as well.
			if (parentSegment_->polyline.size() - 2 == i)
			{
				// See the comment in the previous block for i == 0.
				polyline_.push_back(point2);
			}
		}
	}
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
