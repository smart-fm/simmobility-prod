#pragma once

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"
#include "buffering/Vector2D.hpp"
#include "entities/Person.hpp"
#include <boost/unordered_set.hpp>

using namespace sim_mob;

namespace{

// Return the squared eulidean distance between agent1 and agent2.
double distance(Agent const * agent1, Agent const * agent2)
{
	double x = agent1->xPos - agent2->xPos;
	double y = agent1->yPos - agent2->yPos;
	return x * x + y * y;
}

// Find the agent in the <agents> collection that is closest to <agent>.
Agent const *
nearest_agent(Agent const * agent, boost::unordered_set<Entity const *> const & agents)
{
	Agent const * result = 0;
	double dist = std::numeric_limits<double>::max();
	boost::unordered_set<Entity const *>::const_iterator iter;
	for (iter = agents.begin(); iter != agents.end(); ++iter) {
		Agent const * another_agent = dynamic_cast<Agent const*>(*iter);
		double d = distance(agent, another_agent);
		if (dist > d) {
			// Found a nearer agent.
			dist = d;
			result = another_agent;
		}
	}
	return result;
}

// Return the sum of the width of <lane> and the width of the adjacent lanes on the left
// and right of <lane>.  If there is no lane on the left, add 300 centimeters.  Similarly
// if there is no lane on the right, add 300 centimeters.
centimeter_t getAdjacentLaneWidth(Lane const & lane)
{
	// Find the index of <lane> so that we can find the adjacent lanes.
	RoadSegment const * segment = lane.getRoadSegment();
	std::vector<Lane*> const & lanes = segment->getLanes();
	size_t index = 0;
	while (index < lanes.size()) {
		if (&lane == lanes[index])
			break;
		index++;
	}

	centimeter_t width = lane.getWidth();

	if (0 == index)     // no lane on the left
			{
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

// Return true if <point> is between <p1> and <p2>, even if <point> is not co-linear
// with <p1> and <p2>.
bool isInBetween(Point2D const & point, Point2D const & p1, Point2D const & p2)
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

// Adjust <p1> and <p2> so that <p2> is <distanceInFront> from <position> and
// <p1> is <distanceBehind> from <position>, while retaining the slope of the line
// from <p1> to <p2>.
void adjust(Point2D & p1, Point2D & p2, Point2D const & position, centimeter_t distanceInFront, centimeter_t distanceBehind)
{
	centimeter_t xDiff = p2.getX() - p1.getX();
	centimeter_t yDiff = p2.getY() - p1.getY();

	double h = sqrt(xDiff * xDiff + yDiff * yDiff);
	double t = distanceInFront / h;
	centimeter_t x = position.getX() + t * xDiff;
	centimeter_t y = position.getY() + t * yDiff;
	p2 = Point2D(x, y);

	t = distanceBehind / h;
	x = position.getX() - t * xDiff;
	y = position.getY() - t * xDiff;
	p1 = Point2D(x, y);
}
}
