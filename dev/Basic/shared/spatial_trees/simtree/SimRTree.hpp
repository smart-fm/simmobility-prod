//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

#ifdef SIM_TREE_USE_REBALANCE
	//parameters for re-balance
public:
	//the border of the network
	static long network_minimum_x;
	static long network_minimum_y;
	static long network_maximum_x;
	static long network_maximum_y;

	static int division_x_unit_;
	static int division_y_unit_;

	static double maximum_Rectanle_Weight;
	static double minimum_Rectanle_Border_Length;


	//xuyan:it is hard coded inside
	static int bigtable[1000][1000];

	//if re-balance happens %rebalance_threshold times continuously, then need to rebuild the tree
	int rebalance_threshold;
	double rebalance_load_balance_maximum;
	int checking_frequency;

	int rebalance_counts;
#endif

public:
	SimRTree() : m_root(nullptr), first_leaf(nullptr), leaf_counts(0), leaf_agents_sum(0), unbalance_ratio(0)
#ifdef SIM_TREE_USE_REBALANCE
		,rebalance_counts(0), rebalance_threshold(2), rebalance_load_balance_maximum(0.3), checking_frequency(10)
#endif
{}

	//Typedef to refer to our Bounding boxes.
	typedef RStarBoundingBox<2> BoundingBox;


public:
	/*
	 *build the tree structure using data from the file
	 *File Line Format:
	 *(Parent-Item-ID, Own-ID, BOX_X1, BOX_Y1, BOX_X2, BOX_Y2)
	 */
	void build_tree_structure(const std::string& filename);

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
	void updateAllInternalAgents(std::map<const sim_mob::Agent*, TreeItem*>& connectorMap, const std::set<sim_mob::Agent*>& removedAgentPointers);

	/**
	 *DEBUG purpose
	 */
	void display();

	/**
	 *
	 */
	void measureUnbalance(int time_step);

	/**
	 *
	 */
	void rebalance();

	/**
	 * DEBUG
	 */
	void checkLeaf();

#ifdef SIM_TREE_USE_REBALANCE
	void init_rebalance_settings();
#endif

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
	TreeLeaf* get_rightest_leaf(TreeItem* item);

	//
	TreeLeaf* get_leftest_leaf(TreeItem* item);

	//
	BoundingBox location_bounding_box(Agent * agent);

	//
	BoundingBox OD_bounding_box(Agent * agent);

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
