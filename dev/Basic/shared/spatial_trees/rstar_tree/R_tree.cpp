//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "R_tree.hpp"

#include "entities/Agent.hpp"
#include "spatial_trees/rstar_tree/RStarTree.h"

using namespace sim_mob;

// Return the bounding-box that encloses the agent.
R_tree::BoundingBox bounding_box_r(Agent const * agent)
{
	// The agent has no width nor length.  So the lower-left corner equals to the
	// upper-right corner and is equal to the agent's position.
	R_tree::BoundingBox box;
	box.edges[0].first = box.edges[0].second = agent->xPos;
	box.edges[1].first = box.edges[1].second = agent->yPos;

//	std::cout << "R_tree Agent:xPos" << agent->xPos;
//	std::cout << ",R_tree Agent:yPos" << agent->yPos << std::endl;

	return box;
}

void R_tree::insert(Agent const * agent)
{
	// Insert an agent into the tree, based on the bounding-box that encloses the agent.
	Insert(agent, bounding_box_r(agent));
}

std::vector<Agent const *> R_tree::query(R_tree::BoundingBox const & box) const
{
	// R_tree::AcceptEnclosing functor will call the visitor if the agent is enclosed
	// in <box>.  When called, the visitor saves the agent in <result>.  Therefore, when
	// Query() returns, <result> should contain agents that are located inside <box>.
	std::vector<Agent const *> result;
	const_cast<R_tree*>(this)->Query(R_tree::AcceptEnclosing(box), Collecting_visitor(result));
	// Need to remove the constness of <this> because Query() was not implemented as a
	// const method.
	return result;
}
