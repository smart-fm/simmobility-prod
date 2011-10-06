#include "GeomHelpers.hpp"


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
