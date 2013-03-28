
#include "shared_funcs.hpp"

#include <vector>

#include "buffering/Vector2D.hpp"
#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "geospatial/Point2D.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"


using namespace sim_mob;
using namespace sim_mob::temp_spatial;


double sim_mob::temp_spatial::distance(const Agent * agent1, const Agent* agent2)
{
	double x = agent1->xPos - agent2->xPos;
	double y = agent1->yPos - agent2->yPos;
	return x*x + y*y;
}

const sim_mob::Agent* sim_mob::temp_spatial::nearest_agent(const Agent* agent, const boost::unordered_set<const Entity*>& agents)
{
	const Agent* result = 0;
	double dist = std::numeric_limits<double>::max();
	boost::unordered_set<const Entity*>::const_iterator iter;
	for (iter = agents.begin(); iter != agents.end(); ++iter) {
		const Agent* another_agent = dynamic_cast<const Agent*>(*iter);
		double d = distance(agent, another_agent);
		if (dist > d) {
			// Found a nearer agent.
			dist = d;
			result = another_agent;
		}
	}
	return result;
}

centimeter_t sim_mob::temp_spatial::getAdjacentLaneWidth(const Lane& lane)
{
	// Find the index of <lane> so that we can find the adjacent lanes.
	const RoadSegment* segment = lane.getRoadSegment();
	const std::vector<sim_mob::Lane*>& lanes = segment->getLanes();
	size_t index = 0;
	while (index < lanes.size()) {
		if (&lane == lanes[index])
			break;
		index++;
	}

	centimeter_t width = lane.getWidth();

	if (0 == index) {     // no lane on the left
		width += 300;
		if (lanes.size() - 1 == index)  // no lane on the right
				{
			width += 300;
		} else {
			width += lanes[index + 1]->getWidth();
		}
	} else {
		width += lanes[index - 1]->getWidth();
		if (lanes.size() - 1 == index)  // no lane on the right
				{
			width += 300;
		} else {
			width += lanes[index + 1]->getWidth();
		}
	}
	return width;
}


bool sim_mob::temp_spatial::isInBetween(const Point2D& point, const Point2D& p1, const Point2D& p2)
{
	Vector2D<double> a(p1.getX(), p1.getY());
	Vector2D<double> b(p2.getX(), p2.getY());
	Vector2D<double> c(point.getX(), point.getY());
	Vector2D<double> vec1 = b - a;
	Vector2D<double> vec2 = c - a;
	double dotProduct = vec1 * vec2;
	// If the dot-product is negative, then <point> is "before" <p1> and if it is greater
	// than 1, then <point> is "after" <p2>.
	return (dotProduct > 0.0 && dotProduct < 1.0);
}



void sim_mob::temp_spatial::adjust(Point2D& p1, Point2D& p2, const Point2D& position, centimeter_t distanceInFront, centimeter_t distanceBehind)
{
	centimeter_t xDiff = p2.getX() - p1.getX();
	centimeter_t yDiff = p2.getY() - p1.getY();

	double h = sqrt(xDiff * xDiff + yDiff * yDiff);
	double t = distanceInFront / h;
	centimeter_t x = position.getX() + t * xDiff;
	centimeter_t y = position.getY() + t * yDiff;
	p2 = sim_mob::Point2D(x, y);

	t = distanceBehind / h;
	x = position.getX() - t * xDiff;
	y = position.getY() - t * xDiff;
	p1 = Point2D(x, y);
}

