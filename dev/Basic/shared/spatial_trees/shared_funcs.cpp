//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "shared_funcs.hpp"

#include <vector>

#include "buffering/Vector2D.hpp"
#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "geospatial/network/Point.hpp"
#include "geospatial/network/Lane.hpp"
#include "geospatial/network/RoadSegment.hpp"
#include "geospatial/network/RoadNetwork.hpp"


using namespace sim_mob;
using namespace sim_mob::spatial;

double sim_mob::spatial::distance(const Agent *agent1, const Agent *agent2)
{
	double x = agent1->xPos - agent2->xPos;
	double y = agent1->yPos - agent2->yPos;
	return x * x + y*y;
}

const sim_mob::Agent* sim_mob::spatial::nearest_agent(const Agent *agent, const boost::unordered_set<const Entity *> &agents)
{
	const Agent* result = 0;
	double dist = std::numeric_limits<double>::max();
	boost::unordered_set<const Entity*>::const_iterator iter;
	
	for (iter = agents.begin(); iter != agents.end(); ++iter)
	{
		const Agent *another_agent = dynamic_cast<const Agent *> (*iter);
		double d = distance(agent, another_agent);

		if (dist > d)
		{
			// Found a nearer agent.
			dist = d;
			result = another_agent;
		}
	}
	return result;
}

double sim_mob::spatial::getAdjacentPathWidth(const WayPoint &wayPoint)
{
	double width = 0;

	if (wayPoint.type == WayPoint::LANE)
	{
		const Lane *lane = wayPoint.lane;
		const std::vector<Lane *> &lanes = lane->getParentSegment()->getLanes();

		//Find the index of the lane so that we can find the adjacent lanes.
		size_t index = lane->getLaneIndex();
		width = lane->getWidth();

		if (0 == index)
		{
			//No lane on the left
			width += 0.3;
			if (lanes.size() - 1 == index)
			{
				//No lane on the right
				width += 0.3;
			}
			else
			{
				width += lanes[index + 1]->getWidth();
			}
		}
		else
		{
			width += lanes[index - 1]->getWidth();
			if (lanes.size() - 1 == index)
			{
				//No lane on the right
				width += 0.3;
			}
			else
			{
				width += lanes[index + 1]->getWidth();
			}
		}
	}
	else
	{
		const TurningPath *turning = wayPoint.turningPath;
		width = turning->getWidth();

		//Get the turning group
		const RoadNetwork *network = RoadNetwork::getInstance();
		const TurningGroup *group = network->getById(network->getMapOfIdvsTurningGroups(), turning->getTurningGroupId());

		if(group->getNumTurningPaths() == 1)
		{
			//Only one turning in the group i.e. no lane on the left or right
			width += 0.6;
		}
		else
		{
			//Check if there are other turning paths from the same lane
			const std::map<unsigned int, TurningPath *> *turnings = group->getTurningPaths(turning->getFromLaneId());

			unsigned int turningOnLeft = 0;
			unsigned int turningOnRight = 0;

			if (turnings->size() == 1)
			{
				//Only 1 turning path from this lane, which is the current turning path
				//So the turning to our left is the turning originating from the left lane, and the one to our right is
				//the turning originating from the right lane

				turningOnLeft = turning->getFromLaneId() - 1;
				turningOnRight = turning->getFromLaneId() + 1;
			}
			else
			{
				//Multiple turnings originate from the lane our current turning path originates at
				//So the turning to our left is the turning ending at the left lane, and the one to our right is
				//the turning ending at the right lane

				turningOnLeft = turning->getToLaneId() - 1;
				turningOnRight = turning->getToLaneId() + 1;
			}
			
			//Get the turnings to our left
			turnings = group->getTurningPaths(turningOnLeft);
			if (turnings)
			{
				//Although there are multiple turnings, only 1 is adjacent us. So just use the width of the first one
				width += turnings->begin()->second->getWidth();
			}
			else
			{
				//No turning on the left
				width += 0.3;
			}

			//Get the turnings to our right
			turnings = group->getTurningPaths(turningOnRight);
			if (turnings)
			{
				//Although there are multiple turnings, only 1 is adjacent us. So just use the width of the first one
				width += turnings->begin()->second->getWidth();
			}
			else
			{
				//No turning on the right
				width += 0.3;
			}
		}
	}

	return width;
}

bool sim_mob::spatial::isInBetween(const Point &point, const Point &p1, const Point &p2)
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

void sim_mob::spatial::adjust(Point &p1, Point &p2, const Point &position, double distanceInFront, double distanceBehind)
{
	double xDiff = p2.getX() - p1.getX();
	double yDiff = p2.getY() - p1.getY();

	double h = sqrt(xDiff * xDiff + yDiff * yDiff);
	double t = distanceInFront / h;
	double x = position.getX() + t * xDiff;
	double y = position.getY() + t * yDiff;
	p2 = sim_mob::Point(x, y);

	t = distanceBehind / h;
	x = position.getX() - t * xDiff;
	y = position.getY() - t * yDiff;
	p1 = Point(x, y);
}

