#pragma once

#include <boost/unordered_set.hpp>

#include "buffering/Vector2D.hpp"
#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "geospatial/Lane.hpp"
#include "geospatial/RoadSegment.hpp"

namespace{

// Return the squared eulidean distance between agent1 and agent2.
double distance(sim_mob::Agent const * agent1, sim_mob::Agent const * agent2)
{
	double x = agent1->xPos - agent2->xPos;
	double y = agent1->yPos - agent2->yPos;
	return x * x + y * y;
}

// Find the agent in the <agents> collection that is closest to <agent>.
sim_mob::Agent const *
nearest_agent(sim_mob::Agent const * agent, boost::unordered_set<sim_mob::Entity const *> const & agents)
{
	sim_mob::Agent const * result = 0;
	double dist = std::numeric_limits<double>::max();
	boost::unordered_set<sim_mob::Entity const *>::const_iterator iter;
	for (iter = agents.begin(); iter != agents.end(); ++iter) {
		sim_mob::Agent const * another_agent = dynamic_cast<sim_mob::Agent const*>(*iter);
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
sim_mob::centimeter_t getAdjacentLaneWidth(sim_mob::Lane const & lane)
{
	// Find the index of <lane> so that we can find the adjacent lanes.
	sim_mob::RoadSegment const * segment = lane.getRoadSegment();
	std::vector<sim_mob::Lane*> const & lanes = segment->getLanes();
	size_t index = 0;
	while (index < lanes.size()) {
		if (&lane == lanes[index])
			break;
		index++;
	}

	sim_mob::centimeter_t width = lane.getWidth();

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
bool isInBetween(sim_mob::Point2D const & point, sim_mob::Point2D const & p1, sim_mob::Point2D const & p2)
{
	sim_mob::Vector2D<double> a(p1.getX(), p1.getY());
	sim_mob::Vector2D<double> b(p2.getX(), p2.getY());
	sim_mob::Vector2D<double> c(point.getX(), point.getY());
	sim_mob::Vector2D<double> vec1 = b - a;
	sim_mob::Vector2D<double> vec2 = c - a;
	double dotProduct = vec1 * vec2;
	// If the dot-product is negative, then <point> is "before" <p1> and if it is greater
	// than 1, then <point> is "after" <p2>.
	return (dotProduct > 0.0 && dotProduct < 1.0);
}

// Adjust <p1> and <p2> so that <p2> is <distanceInFront> from <position> and
// <p1> is <distanceBehind> from <position>, while retaining the slope of the line
// from <p1> to <p2>.
void adjust(sim_mob::Point2D & p1, sim_mob::Point2D & p2, sim_mob::Point2D const & position, sim_mob::centimeter_t distanceInFront, sim_mob::centimeter_t distanceBehind)
{
	sim_mob::centimeter_t xDiff = p2.getX() - p1.getX();
	sim_mob::centimeter_t yDiff = p2.getY() - p1.getY();

	double h = sqrt(xDiff * xDiff + yDiff * yDiff);
	double t = distanceInFront / h;
	sim_mob::centimeter_t x = position.getX() + t * xDiff;
	sim_mob::centimeter_t y = position.getY() + t * yDiff;
	p2 = sim_mob::Point2D(x, y);

	t = distanceBehind / h;
	x = position.getX() - t * xDiff;
	y = position.getY() - t * xDiff;
	p1 = sim_mob::Point2D(x, y);
}
}
