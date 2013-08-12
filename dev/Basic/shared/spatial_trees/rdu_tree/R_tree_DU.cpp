/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "R_tree_DU.h"

#include "entities/Agent.hpp"

using namespace sim_mob;

//#ifdef USE_R_DU_TREE

// Return the bounding-box that encloses the agent.
R_tree_DU::BoundingBox bounding_box_du(Agent const * agent)
{
	// The agent has no width nor length.  So the lower-left corner equals to the
	// upper-right corner and is equal to the agent's position.
//	Agent* bu_agent = const_cast<Agent*>(agent);

	R_tree_DU::BoundingBox box;
	box.edges[0].first = box.edges[0].second = agent->xPos;
	box.edges[1].first = box.edges[1].second = agent->yPos;
	return box;
}

//update new location, no need to delete and then insert
void R_tree_DU::update(int object_id, int new_location_x, int new_location_y)
{
	update_(object_id, new_location_x, new_location_y);
}

//remove an Agent
void R_tree_DU::remove(Agent const * agent)
{
	remove_(agent->getId());
}

bool R_tree_DU::has_one_agent(int agent_id)
{
	return has_one_object(agent_id);
}

void R_tree_DU::insert(Agent const * agent)
{
	// Insert an agent into the tree, based on the bounding-box that encloses the agent.
	Insert(agent, bounding_box_du(agent), agent->getId());
}

std::vector<Agent const *> R_tree_DU::query(R_tree_DU::BoundingBox const & box) const
{
	// R_tree_DU::AcceptEnclosing functor will call the visitor if the agent is enclosed
	// in <box>.  When called, the visitor saves the agent in <result>.  Therefore, when
	// Query() returns, <result> should contain agents that are located inside <box>.
	std::vector<Agent const *> result;

	//const_cast<R_tree_DU*> (this)->Query(R_tree_DU::AcceptEnclosing(box), Collecting_visitor(result));
	Query(R_tree_DU::AcceptEnclosing(box), Collecting_visitor(result));

	// Need to remove the constness of <this> because Query() was not implemented as a
	// const method.
	return result;
}

//#endif
