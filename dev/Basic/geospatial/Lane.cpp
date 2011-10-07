/* Copyright Singapore-MIT Alliance for Research and Technology */

#include <math.h>
#include <assert.h>

#include "Lane.hpp"

namespace sim_mob
{

namespace
{
    // Return the distance between the middle of the lane specified by <thisLane> and the middle
    // of the road-segment specified by <segment>; <thisLane> is one of the lanes in <segment>.
    double middle(const Lane& thisLane, const RoadSegment& segment)
    {
        double w = segment.width / 2.0;
        if (segment.width == 0)
        {
            double width = 0;
            const std::vector<Lane*>& lanes = segment.getLanes();
            for (size_t i = 0; i < lanes.size(); i++)
            {
                width += lanes[i]->getWidth();
            }
            w = width / 2.0;
        }

        const std::vector<Lane*>& lanes = segment.getLanes();
        for (size_t i = 0; i < lanes.size(); i++)
        {
            const Lane* lane = lanes[i];
            if (lane != &thisLane)
                w -= lane->getWidth();
            else
            {
                w -= (lane->getWidth() / 2.0);
                return w;
            }
        }
        assert(false); // We shouldn't reach here.
        return w;
    }

    // Return the point that is perpendicular to the line that passes through <p> and 
    // is sloping <dx> horizontally and <dy> vertically.  The distance between <p> and the
    // returned point is <w>.  If <w> is negative, the returned point is "above" the line;
    // otherwise it is below the line.
    Point2D getSidePoint(const Point2D& p, int dx, int dy, double w)
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
    }

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
	double w = middle(*this, *parentSegment_);

	//Set the width if it hasn't been set
	if (width_==0) {
		width_ = parentSegment_->width/parentSegment_->getLanes().size();
	}

	for (size_t i = 0; i < parentSegment_->polyline.size() - 1; i++)
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
			Point2D p = getSidePoint(p1, dx, dy, w);
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
			Point2D point1 = getSidePoint(p0, dx1, dy1, w);
			Point2D point2 = getSidePoint(p2, dx2, dy2, w);
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
