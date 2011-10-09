#include "GeomHelpers.hpp"

#include <stdexcept>
#include <limits>

#include "../geospatial/Point2D.hpp"
#include "../geospatial/aimsun/Lane.hpp"
#include "../geospatial/aimsun/Node.hpp"
#include "../geospatial/aimsun/Section.hpp"
#include "../geospatial/aimsun/Crossing.hpp"

using namespace sim_mob;


double sim_mob::dist(double x1, double y1, double x2, double y2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	return sqrt(dx*dx + dy*dy);
}

double sim_mob::dist(const aimsun::Crossing* c1, const aimsun::Crossing* c2) {
	return dist(c1->xPos, c1->yPos, c2->xPos, c2->yPos);
}
double sim_mob::dist(const aimsun::Lane* ln, const aimsun::Node* nd) {
	return dist(ln->xPos, ln->yPos, nd->xPos, nd->yPos);
}
double sim_mob::dist(const aimsun::Lane* ln1, const aimsun::Lane* ln2) {
	return dist(ln1->xPos, ln1->yPos, ln2->xPos, ln2->yPos);
}
double sim_mob::dist(const aimsun::Node* n1, const aimsun::Node* n2) {
	return dist(n1->xPos, n1->yPos, n2->xPos, n2->yPos);
}
double sim_mob::dist(const Point2D* p1, const Point2D* p2) {
	return dist(p1->getX(), p1->getY(), p2->getX(), p2->getY());
}


bool sim_mob::lineContains(double ax, double ay, double bx, double by, double cx, double cy)
{
	//Check if the dot-product is >=0 and <= the squared distance
	double dotProd = (cx - ax) * (bx - ax) + (cy - ay)*(by - ay);
	double sqLen = (bx - ax)*(bx - ax) + (by - ay)*(by - ay);
	return dotProd>=0 && dotProd<=sqLen;

}
bool sim_mob::lineContains(const aimsun::Crossing* p1, const aimsun::Crossing* p2, double xPos, double yPos)
{
	return lineContains(p1->xPos, p1->yPos, p2->xPos, p2->yPos, xPos, yPos);
}
bool sim_mob::lineContains(const aimsun::Section* sec, double xPos, double yPos)
{
	return lineContains(sec->fromNode->xPos, sec->fromNode->yPos, sec->toNode->xPos, sec->toNode->yPos, xPos, yPos);
}



bool sim_mob::PointIsLeftOfVector(double ax, double ay, double bx, double by, double cx, double cy)
{
	//Via cross-product
	return ((bx - ax)*(cy - ay) - (by - ay)*(cx - ax)) > 0;
}
bool sim_mob::PointIsLeftOfVector(const aimsun::Node* vecStart, const aimsun::Node* vecEnd, const aimsun::Lane* point)
{
	return PointIsLeftOfVector(vecStart->xPos, vecStart->yPos, vecEnd->xPos, vecEnd->yPos, point->xPos, point->yPos);
}
bool sim_mob::PointIsLeftOfVector(const DynamicVector& vec, const aimsun::Lane* point)
{
	return PointIsLeftOfVector(vec.getX(), vec.getY(), vec.getEndX(), vec.getEndY(), point->xPos, point->yPos);
}



Point2D sim_mob::LineLineIntersect(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	//If the points are all on top of each other, return any pair of points
	if ((x1==x2 && x2==x3 && x3==x4) && (y1==y2 && y2==y3 && y3==y4)) {
		return Point2D(x1, y1);
	}

	//Check if we're doomed to failure (parallel lines) Compute some intermediate values too.
	double denom = (x1-x2)*(y3-y4) - (y1-y2)*(x3-x4);
	if (denom==0) {
		//NOTE: For now, I return Double.MAX,Double.MAX. C++11 will introduce some help for this,
		//      or we could find a better way to do it....
		return Point2D(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
		//throw std::runtime_error("Can't compute line-line intersection: division by zero.");
	}
	double co1 = x1*y2 - y1*x2;
	double co2 = x3*y4 - y3*x4;

	//Results!
	double xRes = (co1*(x3-x4) - co2*(x1-x2)) / denom;
	double yRes = (co1*(y3-y4) - co2*(y1-y2)) / denom;
	return Point2D(xRes, yRes);
}
Point2D sim_mob::LineLineIntersect(const aimsun::Crossing* const p1, const aimsun::Crossing* p2, const aimsun::Section* sec)
{
	return LineLineIntersect(p1->xPos,p1->yPos, p2->xPos,p2->yPos, sec->fromNode->xPos,sec->fromNode->yPos, sec->toNode->xPos,sec->toNode->yPos);
}
Point2D sim_mob::LineLineIntersect(const DynamicVector& v1, const DynamicVector& v2)
{
	return LineLineIntersect(v1.getX(), v1.getY(), v1.getEndX(), v1.getEndY(), v2.getX(), v2.getY(), v2.getEndX(), v2.getEndY());
}
Point2D sim_mob::LineLineIntersect(const Point2D& p1, const Point2D& p2, const Point2D& p3, const Point2D& p4)
{
	return LineLineIntersect(p1.getX(),p1.getY(), p2.getX(),p2.getY(), p3.getX(),p3.getY(), p4.getX(),p4.getY());
}




