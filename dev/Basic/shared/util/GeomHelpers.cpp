//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "GeomHelpers.hpp"

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <limits>

#include "boost/lexical_cast.hpp"
#include "geospatial/simmobility_network/Point.hpp"

using namespace sim_mob;
using std::vector;
using std::string;

double sim_mob::dist(Point pt1, Point pt2)
{
	double dx = pt2.getX() - pt1.getX();
	double dy = pt2.getY() - pt1.getY();
	return sqrt(dx * dx + dy * dy);
}

double sim_mob::dist(double x1, double y1, double x2, double y2)
{
	return dist(Point(x1,y1), Point(x2,y2));
}

///////////////////////////////////////////////////////////////////////////
// Template implementations for dist()
///////////////////////////////////////////////////////////////////////////

template <class T> double sim_mob::dist(double x1, double y1, const T& point2)
{
	return dist(Point(x1, y1), get_distarg(point2));
}

template <class T> double sim_mob::dist(const T& point1, double x2, double y2)
{
	return dist(get_distarg(point1), Point(x2, y2));
}

template <class T, class Y> double sim_mob::dist(const T& point1, const Y& point2)
{
	return dist(get_distarg(point1), get_distarg(point2));
}

Point sim_mob::normal_intersect(const Point& pt, const DynamicVector& line)
{
	//First, retrieve the coordinates of the point closest to 'pt' from 'line'.
	//Using the equation of the line, we solve for 'u'.
	double lhs = (pt.getX() - line.getX()) * (line.getEndX() - line.getX());
	double rhs = (pt.getY() - line.getY()) * (line.getEndY() - line.getY());
	double denom = line.getMagnitude() * line.getMagnitude();

	if (denom == 0)
	{
		std::stringstream msg;
		msg << "Cannot determine the normal intersection: points are co-incidental.";
		msg << "\n  Point: (" << pt.getX() << "," << pt.getY() << ")";
		msg << "\n  Line: (" << line.getX() << "," << line.getY() << ") => (" << line.getEndX() << "," << line.getEndY() << ")";
		throw std::runtime_error(msg.str().c_str());
	}
	
	double u = (lhs + rhs) / denom;

	//Now substitute it to get the actual point.
	int x = (line.getX() + u * (line.getEndX() - line.getX()));
	int y = (line.getY() + u * (line.getEndY() - line.getY()));
	Point res(x, y);

	//We can also test that this point is actually between the start and end. I'm not sure if the
	// dot product approach above guarantees this or not.
	if ((dist(res.getX(), res.getY(), line.getX(), line.getY()) > line.getMagnitude()) ||
		(dist(res.getX(), res.getY(), line.getEndX(), line.getEndY()) > line.getMagnitude()))
	{
		std::stringstream msg;
		msg << "Cannot determine the normal intersection: point is too far outside the line.";
		msg << "\n  Point: (" << pt.getX() << "," << pt.getY() << ")";
		msg << "\n  Line: (" << line.getX() << "," << line.getY() << ") => (" << line.getEndX() << "," << line.getEndY() << ")";
		throw std::runtime_error(msg.str().c_str());
	}

	return res;
}


bool sim_mob::lineContains(double ax, double ay, double bx, double by, double cx, double cy)
{
	//Check if the dot-product is >=0 and <= the squared distance
	double dotProd = (cx - ax) * (bx - ax) + (cy - ay) * (by - ay);
	double sqLen = (bx - ax) * (bx - ax) + (by - ay) * (by - ay);
	return dotProd >= 0 && dotProd <= sqLen;

}

bool sim_mob::PointIsLeftOfVector(double ax, double ay, double bx, double by, double cx, double cy)
{
	//Via cross-product
	return ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax)) > 0;
}

bool sim_mob::PointIsLeftOfVector(const DynamicVector& vec, double x, double y)
{
	return PointIsLeftOfVector(vec.getX(), vec.getY(), vec.getEndX(), vec.getEndY(), x, y);
}

Point sim_mob::LineLineIntersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	//If the points are all on top of each other, return any pair of points
	if ((x1 == x2 && x2 == x3 && x3 == x4) && (y1 == y2 && y2 == y3 && y3 == y4))
	{
		return Point(x1, y1);
	}

	//Check if we're doomed to failure (parallel lines) Compute some intermediate values too.
	double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

	if (denom == 0)
	{
		//NOTE: For now, I return Double.MAX,Double.MAX. C++11 will introduce some help for this,
		//      or we could find a better way to do it....
		return Point(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	}
	
	double co1 = x1 * y2 - y1 * x2;
	double co2 = x3 * y4 - y3 * x4;

	//Results!
	double xRes = (co1 * (x3 - x4) - co2 * (x1 - x2)) / denom;
	double yRes = (co1 * (y3 - y4) - co2 * (y1 - y2)) / denom;
	
	return Point(static_cast<int> (xRes), static_cast<int> (yRes));
}

Point sim_mob::LineLineIntersect(const DynamicVector& v1, const DynamicVector& v2)
{
	return LineLineIntersect(v1.getX(), v1.getY(), v1.getEndX(), v1.getEndY(), v2.getX(), v2.getY(), v2.getEndX(), v2.getEndY());
}

Point sim_mob::LineLineIntersect(const DynamicVector& v1, const Point& p3, const Point& p4)
{
	return LineLineIntersect(v1.getX(), v1.getY(), v1.getEndX(), v1.getEndY(), p3.getX(), p3.getY(), p4.getX(), p4.getY());
}

Point sim_mob::LineLineIntersect(const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	return LineLineIntersect(p1.getX(), p1.getY(), p2.getX(), p2.getY(), p3.getX(), p3.getY(), p4.getX(), p4.getY());
}

Point sim_mob::ProjectOntoLine(const Point& pToProject, const Point& pA, const Point& pB)
{
	double dotProductToPoint = (pToProject.getX() - pA.getX()) * (pB.getX() - pA.getX()) + (pToProject.getY() - pA.getY()) * (pB.getY() - pA.getY());
	double dotProductOfLine = (pB.getX() - pA.getX()) * (pB.getX() - pA.getX()) + (pB.getY() - pA.getY()) * (pB.getY() - pA.getY());
	double dotRatio = dotProductToPoint / dotProductOfLine;

	Point AB(pB.getX() - pA.getX(), pB.getY() - pA.getY());
	Point ABscaled(AB.getX() * dotRatio, AB.getY() * dotRatio);

	return Point(pA.getX() + ABscaled.getX(), pA.getY() + ABscaled.getY());
}

Point sim_mob::getSidePoint(const Point& origin, const Point& direction, double magnitude)
{
	DynamicVector dv(origin.getX(), origin.getY(), direction.getX(), direction.getY());
	dv.flipNormal(false).scaleVectTo(magnitude).translateVect();
	return Point(dv.getX(), dv.getY());
}

namespace 
{
	// Return the point that is perpendicular (with magnitude <magnitude>) to the vector that begins at <origin> and
	// passes through <direction>. This point is left of the vector if <magnitude> is positive.
	Point getSidePoint(const Point& origin, const Point& direction, double magnitude)
	{
		DynamicVector dv(origin.getX(), origin.getY(), direction.getX(), direction.getY());
		dv.flipNormal(false).scaleVectTo(magnitude).translateVect();
		return Point(dv.getX(), dv.getY());
	}

	// Return the intersection of the vectors (pPrev->pCurr) and (pNext->pCurr) when extended by "magnitude"
	Point calcCurveIntersection(const Point& pPrev, const Point& pCurr, const Point& pNext, double magnitude)
	{
		//Get an estimate on the maximum distance. This isn't strictly needed, since we use the line-line intersection formula later.
		double maxDist = dist(pPrev, pNext);

		//Get vector 1.
		DynamicVector dvPrev(pPrev.getX(), pPrev.getY(), pCurr.getX(), pCurr.getY());
		dvPrev.translateVect().flipNormal(false).scaleVectTo(magnitude).translateVect();
		dvPrev.flipNormal(true).scaleVectTo(maxDist);

		//Get vector 2
		DynamicVector dvNext(pNext.getX(), pNext.getY(), pCurr.getX(), pCurr.getY());
		dvNext.translateVect().flipNormal(true).scaleVectTo(magnitude).translateVect();
		dvNext.flipNormal(false).scaleVectTo(maxDist);

		//Compute their intersection. We use the line-line intersection formula because the vectors
		//won't intersect for acute angles.
		return LineLineIntersect(dvPrev, dvNext);
	}
} //End anon namespace


vector<Point> sim_mob::ShiftPolyline(const vector<Point>& orig, double shiftAmt, bool shiftLeft)
{
	//Deal with right-shifts in advance.
	if (!shiftLeft)
	{
		shiftAmt *= -1;
	}

	//Sanity check
	if (orig.size() < 2)
	{
		throw std::runtime_error("Can't shift an empty poly-line or one of size 1.");
	}

	//Push back the first point.
	vector<Point> res;
	res.push_back(getSidePoint(orig.front(), orig.back(), shiftAmt));

	//Push back the last point
	res.push_back(getSidePoint(orig.back(), orig.front(), -shiftAmt));
	return res;
}

//add by xuyan
Point sim_mob::getMiddlePoint(const Point* start_point, const Point* end_point, double offset)
{
	double distance = dist(start_point->getX(), start_point->getY(), end_point->getX(), end_point->getY());
	double location_x = start_point->getX() + (offset * 100) / distance * (end_point->getX() - start_point->getX());
	double location_y = start_point->getY() + (offset * 100) / distance * (end_point->getY() - start_point->getY());

	return Point(location_x, location_y);
}

//add by xuyan
//To determine the status of a point (xp,yp) consider a horizontal ray emanating from (xp,yp) and to the right.
//If the number of times this ray intersects the line segments making up the polygon is even then the point is outside the polygon.
//Whereas if the number of intersections is odd then the point (xp,yp) lies inside the polygon.
bool sim_mob::PointInsidePolygon(const Point* polygon, int N, const Point p)
{
	int counter = 0;
	int i;
	double xinters;
	Point p1, p2;

	p1 = polygon[0];
	for (i = 1; i <= N; i++)
	{
		p2 = polygon[i % N];
		if (p.getY() > std::min(p1.getY(), p2.getY()))
		{
			if (p.getY() <= std::max(p1.getY(), p2.getY()))
			{
				if (p.getX() <= std::max(p1.getX(), p2.getX()))
				{
					if (p1.getY() != p2.getY())
					{
						xinters = (p.getY() - p1.getY()) * (p2.getX() - p1.getX()) / (p2.getY() - p1.getY()) + p1.getX();
						if (p1.getX() == p2.getX() || p.getX() <= xinters)
							counter++;
					}
				}
			}
		}
		p1 = p2;
	}

	//if there is even intersections
	if (counter % 2 == 0)
	{
		return (false);
	}
	//if there is odd intersections
	else
	{
		return (true);
	}
}

Point sim_mob::parse_point(const string& str)
{
	//We need at least "1,2"
	if (str.length() < 3)
	{
		throw std::runtime_error("Can't parse a point from a (mostly) empty source string.");
	}

	//Does it match the pattern?
	size_t commaPos = str.find(',');
	
	if (commaPos == string::npos)
	{
		throw std::runtime_error("Can't parse a point with no comma separator.");
	}

	//Allow for an optional parentheses
	size_t StrOffset = 0;
	if (str[0] == '(' && str[str.length() - 1] == ')')
	{
		//Parentheses require at least "(1,2)"
		if (str.length() < 5)
		{
			throw std::runtime_error("Can't parse a point from a (mostly) empty source string.");
		}
		
		StrOffset = 1;
	}

	//Try to parse its substrings
	int xPos, yPos;
	std::istringstream(str.substr(StrOffset, commaPos - StrOffset)) >> xPos;
	std::istringstream(str.substr(commaPos + 1, str.length()-(commaPos + 1) - StrOffset)) >> yPos;

	return Point(xPos, yPos);
}

std::pair<uint32_t, uint32_t> sim_mob::parse_point_pair(const std::string& src)
{
	std::pair<uint32_t, uint32_t> res;
	std::stringstream curr;
	for (std::string::const_iterator it=src.begin(); it != src.end(); it++)
	{
		//Skip whitespace, parentheses
		const char c = *it;
		if (c == ' ')
		{
			continue;
		}
		if (c == '(' && it == src.begin())
		{
			continue;
		}
		if (c == ')' && (it + 1) == src.end())
		{
			continue;
		}

		//Append digits?
		if (c >= '0' && c <= '9')
		{
			curr << c;
			continue;
		}

		//Done with the X?
		if (c == ',')
		{
			if (curr.str().empty())
			{
				throw std::runtime_error("Can't parse point; empty X");
			}
			res.first = boost::lexical_cast<uint32_t>(curr.str());
			curr.str("");
		}
	}

	//Done with the Y
	if (curr.str().empty())
	{
		throw std::runtime_error("Can't parse point; empty Y");
	}
	res.second = boost::lexical_cast<uint32_t>(curr.str());

	return res;
}

////Specialisation implementations

template <> Point sim_mob::get_distarg(const Point& item)
{
	return Point(item.getX(), item.getY());
}

template <class T> Point sim_mob::get_distarg(T* item)
{
	return get_distarg(*item);
}