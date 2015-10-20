//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/**
 * There are three different Tree-based Spatial Indexes implemented in SimMobility: R*-Tree, R-DU Tree and Sim-Tree.
 * R*-Tree is the default choice
 * R-DU Tree is used for research comparison
 * Sim-Tree is the optimal choice
 */

#pragma once

#include <map>
#include <vector>

#include "util/LangHelpers.hpp"
#include "spatial_trees/rstar_tree/RStarBoundingBox.hpp"
#include "spatial_trees/spatial_tree_include.hpp"

//Note: this class is designed for SimMobility.
//Agent class has been compiled into Tree Structure

/**
 * Tree Node Definition
 */

namespace sim_mob
{

//Forward declarations.
class Entity;
class Agent;
//Forward declare structs used in this class.
struct TreeItem;
struct TreeLeaf;
struct TreeNode;


/**
 * Tree Definition
 */

class SimRTree {
private:
	TreeNode* m_root;
	TreeLeaf* first_leaf;

private:
	long leaf_counts;
	double leaf_agents_sum;
	double unbalance_ratio;

public:
	static long network_minimum_x;
	static long network_minimum_y;
	static long network_maximum_x;
	static long network_maximum_y;

	static int division_x_unit_;
	static int division_y_unit_;

	static double maximum_Rectanle_Weight;
	static double minimum_Rectanle_Border_Length;


	//it is to divide the network into a 1000*1000 2-D matrix
	static int bigtable[DIVIDE_NETWORK_X_INTO_CELLS][DIVIDE_NETWORK_Y_INTO_CELLS];

	//if re-balance happens %rebalance_threshold times continuously, then need to rebuild the tree
	int rebalance_threshold;
	double rebalance_load_balance_maximum;
	int checking_frequency;

	int rebalance_counts;

public:
	SimRTree() : m_root(nullptr), first_leaf(nullptr), leaf_counts(0), leaf_agents_sum(0), unbalance_ratio(0)
		,rebalance_counts(0), rebalance_threshold(2), rebalance_load_balance_maximum(0.3), checking_frequency(10)
{}

	//Typedef to refer to our Bounding boxes.
	typedef RStarBoundingBox<2> BoundingBox;


public:
	/*
	 *build the tree structure using data from the file
	 *File Line Format:
	 *(Parent-Item-ID, Own-ID, BOX_X1, BOX_Y1, BOX_X2, BOX_Y2)
	 */
	void buildTreeStructure(const std::string& filename);

	/**
	 * build the tree structure by re-using the road network
	 */
	void buildTreeStructure();

	/**
	 * Assumption:
	 * The tree contains the whole Road Network;
	 * The parameter "connectorMap" is passed in from the parent SimAuraManager. The SimRTree updates this instead of modifying the Agent directly.
	 */
	void insertAgent(Agent * agent, std::map<const sim_mob::Agent*, TreeItem*>& connectorMap);

	//	/**
	//	 *Currently, when new vehicle is insert, the original node is used for locationing.
	//   * The parameter "connectorMap" is passed in from the parent SimAuraManager. The SimRTree updates this instead of modifying the Agent directly.
	//	 */
	void insertAgentBasedOnOD(Agent * agent, std::map<const sim_mob::Agent*, TreeItem*>& connectorMap);

	/**
	 * Return all agents in the box, called by each agent each time step;
	 * Thus, should be efficient
	 */
	std::vector<Agent const *> rangeQuery(BoundingBox & box) const;

	/**
	 * Start Query From Somewhere, not from the root.
	 * Bottom_Up Query
	 */
	std::vector<Agent const*> rangeQuery(SimRTree::BoundingBox & box, TreeItem* item) const;

	/**
	 * Automatically Update Internal Agents' Locations
	 * The parameter "connectorMap" is passed in from the parent SimAuraManager. The SimRTree updates this instead of modifying the Agent directly.
	 * Note: The pointers in removedAgentPointers will be deleted after this time tick; do *not*
	 *       save them anywhere.
	 */
	void updateAllInternalAgents(std::map<const sim_mob::Agent*, TreeItem*>& connectorMap, const std::set<sim_mob::Entity*>& removedAgentPointers);

	/**
	 *DEBUG purpose
	 */
	void display();

	/**
	 *
	 */
	void measureUnbalance(int time_step, std::map<const sim_mob::Agent*, TreeItem*>& agent_connector_map);

	/**
	 *
	 */
	void rebalance(std::map<const sim_mob::Agent*, TreeItem*>& agent_connector_map);

	/**
	 * DEBUG
	 */
	void checkLeaf();

	void initRebalanceSettings();

private:
	//release memory
	void releaseTreeMemory();

	//
	void releaseNodeMemory(TreeItem* item);

	//
	void countLeaf();

	//
	void addNodeToFather(std::size_t father_id, TreeNode * from_node, TreeNode* new_node);

	//
	void addLeafToFather(std::size_t father_id, TreeNode * from_node, TreeLeaf* new_node);

	//
	void connectLeaf();

	//
	void connectLeafs(TreeNode * one_node);

	//
	BoundingBox locationBoundingBox(Agent * agent);

	//
	BoundingBox ODBoundingBox(Agent * agent);

	//
	//The parameter "connectorMap" is passed in from the parent SimAuraManager. The SimRTree updates this instead of modifying the Agent directly.
	void insertAgentEncloseBox(Agent * agent, BoundingBox & agent_box, TreeItem* item, std::map<const sim_mob::Agent*, TreeItem*>& connectorMap);

	//
	void display(TreeItem* item, int level);
};



struct TreeItem {
	SimRTree::BoundingBox bound;
	bool is_leaf;
	std::size_t item_id;

	//For Bottom-Up Query
	TreeItem* father;
};

struct TreeNode: TreeItem {
	std::vector<TreeItem*> items;
};

struct TreeLeaf: TreeItem {
	std::vector<Agent*> agent_buffer;
	TreeLeaf* next;
};



}
