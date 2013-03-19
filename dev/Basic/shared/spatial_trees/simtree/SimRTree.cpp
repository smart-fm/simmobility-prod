/* Copyright Singapore-MIT Alliance for Research and Technology */

#include "SimRTree.h"

#include <string>
#include <limits>
#include <cmath>

#include "entities/Entity.hpp"
#include "entities/Agent.hpp"
#include "entities/Person.hpp"

using namespace sim_mob;

#ifdef USE_REBALANCE

long SimRTree::network_minimum_x = 0;
long SimRTree::network_minimum_y = 0;
long SimRTree::network_maximum_x = 0;
long SimRTree::network_maximum_y = 0;

int SimRTree::division_x_unit_ = 0;
int SimRTree::division_y_unit_ = 0;

//int SimRTree::map_division_x_max = 1000;
//int SimRTree::map_division_y_max = 1000;

int SimRTree::bigtable[1000][1000];

const int sim_mob::SimRTree::checking_frequency = 3;

struct BigTableUpdate: std::unary_function<const Entity *, void>
{
	void operator()(const Entity * item)
	{
		const Person* person = dynamic_cast<const Person*>(item);
		if (person && person->xPos > 0 && person->yPos > 0) {

			int x = (person->xPos - SimRTree::network_minimum_x) / SimRTree::division_x_unit_;
			int y = (person->yPos - SimRTree::network_minimum_y) / SimRTree::division_y_unit_;

//			std::cout << person->xPos << "," << person->yPos << "," << x << "," << y << std::endl;

			SimRTree::bigtable[y][x]++;
		}
	}
};
#endif

/**
 *
 */

struct Collecting_Visitor_sim
{
	const bool ContinueVisiting;
	std::vector<Agent const *> & array; // must be a reference.
	SimRTree::BoundingBox & box;

	explicit Collecting_Visitor_sim(std::vector<Agent const *> & array_, SimRTree::BoundingBox & box_) :
			ContinueVisiting(true), array(array_), box(box_)
	{
	}

	// When called, the visitor saves the agent in <array>.
	bool operator()(TreeLeaf * leaf)
	{
		//		std::cout << "FFF"  << std::endl;

		if (box.encloses(leaf->bound)) {
			array.insert(array.end(), leaf->agent_buffer.begin(), leaf->agent_buffer.end());
		} else {
			for (std::vector<Agent*>::iterator itr = leaf->agent_buffer.begin(); itr != leaf->agent_buffer.end(); itr++) {
				Agent * one_agent = (*itr);

				if (box.enclose_point(one_agent->xPos, one_agent->yPos)) {
					array.push_back(one_agent);
				}
			}
		}
		//		array.push_back(leaf->agent_buffer);
		return true;
	}
};

struct AcceptEnclosing
{
	const SimRTree::BoundingBox &m_bound;
	explicit AcceptEnclosing(const SimRTree::BoundingBox &bound) :
			m_bound(bound)
	{
	}

	bool operator()(const TreeNode * node) const
	{
		return m_bound.overlaps(node->bound);
	}

	bool operator()(const TreeLeaf * leaf) const
	{
		//		std::cout << "EEE"  << std::endl;
		//		std::cout << m_bound.overlaps(leaf->bound)  << std::endl;
		//		std::cout << m_bound.encloses(leaf->bound)  << std::endl;
		return m_bound.overlaps(leaf->bound);
	}

	//private:
	//	AcceptEnclosing()
	//	{
	//	}
};

// visits a node if necessary
struct VisitFunctor: std::unary_function<const TreeItem *, void>
{
	const AcceptEnclosing &accept;
	Collecting_Visitor_sim &visit;

	explicit VisitFunctor(const AcceptEnclosing &a, Collecting_Visitor_sim &v) :
			accept(a), visit(v)
	{
	}

	void operator()(TreeItem * item)
	{
		TreeLeaf * leaf = static_cast<TreeLeaf*>(item);

		//		std::cout << "DDD"  << std::endl;
		//		std::cout << "DDD:" << accept(leaf)  << std::endl;

		if (accept(leaf)) {
			//			std::cout << "DDD2222222222"  << std::endl;
			visit(leaf);
		}
	}
};

//loop the tree
struct QueryFunctor: std::unary_function<const TreeItem, void>
{
	const AcceptEnclosing &accept;
	Collecting_Visitor_sim& visitor;

	explicit QueryFunctor(AcceptEnclosing a, Collecting_Visitor_sim v) :
			accept(a), visitor(v)
	{
	}

	void operator()(TreeItem * item)
	{
		if (item->is_leaf) {
			//			std::cout << "CCC"  << std::endl;

			TreeLeaf * leaf = static_cast<TreeLeaf *>(item);

			//			if (visitor.ContinueVisiting && accept(leaf))
			VisitFunctor(accept, visitor)(leaf);
		} else {
			//			std::cout << "BBB"  << std::endl;

			TreeNode * node = static_cast<TreeNode *>(item);

			if (visitor.ContinueVisiting && accept(node))
				for_each(node->items.begin(), node->items.end(), *this);
		}
	}
};

/**
 *Functor that used to rebuild the tree
 */
#ifdef USE_REBALANCE
struct DivideHorizontalDifference
{
	explicit DivideHorizontalDifference()
	{
	}

	double operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int cut_index)
	{
		double cost_A = 0;
		double cost_B = 0;

		for (int i = from_values_y; i < to_values_y; i++) {
			for (int j = from_values_x; j < to_values_x; j++) {
				if (i < cut_index) {
					cost_A += SimRTree::bigtable[i][j];
				} else {
					cost_B += SimRTree::bigtable[i][j];
				}
			}
		}

		return cost_A - cost_B;
	}
};

struct DivideVerticalDifference
{
	explicit DivideVerticalDifference()
	{
	}

	double operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int cut_index)
	{
		double cost_A = 0;
		double cost_B = 0;

		for (int i = from_values_y; i < to_values_y; i++) {
			for (int j = from_values_x; j < to_values_x; j++) {
				if (j < cut_index) {
					cost_A += SimRTree::bigtable[i][j];
				} else {
					cost_B += SimRTree::bigtable[i][j];
				}
			}
		}

		return cost_A - cost_B;
	}
};

struct DivideHorizontal
{
	explicit DivideHorizontal()
	{
	}

	void operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int* best_cut_point, double * best_cost)
	{
		//too small to cut
		if (to_values_y <= from_values_y + 1)
		return;

		int to_values_y_ = to_values_y;
		int from_values_y_ = from_values_y;
		double best_difference = std::numeric_limits<double>::max();
		int best_cut = -1;

		int middle_cut = 0;
		bool reach_border = false;

		while (to_values_y_ > from_values_y_ && reach_border == false) {
			if (to_values_y_ == from_values_y_ + 1)
			reach_border = true;

			middle_cut = (to_values_y_ + from_values_y_) / 2;
			double difference = DivideHorizontalDifference()(from_values_x, from_values_y, to_values_x, to_values_y, middle_cut);

//			std::cout << "difference" << difference << std::endl;

			if (difference == 0) {
				best_difference = 0;
				best_cut = middle_cut;
				break;
			} else if (difference < 0) {
				from_values_y_ = middle_cut;

				if (best_difference > -1 * difference) {
					best_cut = middle_cut;
					best_difference = -1 * difference;
				}
			} else if (difference > 0) {
				to_values_y_ = middle_cut;

				if (best_difference > difference) {
					best_cut = middle_cut;
					best_difference = difference;
				}
			}
		}

		*best_cut_point = best_cut;
		*best_cost = best_difference;
	}
};

struct DivideVertical
{
	explicit DivideVertical()
	{
	}

	void operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int* best_cut_point, double * best_cost)
	{
		//too small to cut
		if (to_values_x <= from_values_x + 1)
		{
//			std::cout << "to_values_x " << to_values_x << " from_values_x" << from_values_x << std::endl;
//			std::cout << "to_values_y " << to_values_x << " from_values_y" << from_values_x << std::endl;

			return;
		}

		int to_values_x_ = to_values_x;
		int from_values_x_ = from_values_x;
		double best_difference = std::numeric_limits<double>::max();
		int best_cut = -1;

		int middle_cut = 0;
		bool reach_border = false;

		while (to_values_x_ > from_values_x_ && reach_border == false) {
			if (to_values_x_ == from_values_x_ + 1)
			reach_border = true;

			middle_cut = (to_values_x_ + from_values_x_) / 2;
			double difference = DivideVerticalDifference()(from_values_x, from_values_y, to_values_x, to_values_y, middle_cut);

//			std::cout << "difference" << difference << std::endl;

			if (difference == 0) {
				best_difference = 0;
				best_cut = middle_cut;
				break;
			} else if (difference < 0) {
				from_values_x_ = middle_cut;

				if (best_difference > -1 * difference) {
					best_cut = middle_cut;
					best_difference = -1 * difference;
				}
			} else if (difference > 0) {
				to_values_x_ = middle_cut;

				if (best_difference > difference) {
					best_cut = middle_cut;
					best_difference = difference;
				}
			}
		}

		*best_cut_point = best_cut;
		*best_cost = best_difference;
	}
};

struct RebuildSimTreeNodeFunctor
{
	//	int from_values_x;
	//	int from_values_y;
	//	int to_values_x;
	//	int to_values_y;
	//	int current_level;
	//	int target_level;
	//	TreeNode * parent;

	explicit RebuildSimTreeNodeFunctor()
	{
	}

	void operator()(int from_values_x, int from_values_y, int to_values_x, int to_values_y, int current_level, int target_level, TreeNode * parent_)
	{
		//too small to cut, so make a leaf
		if (to_values_x - from_values_x <= 1 && to_values_y - from_values_y <= 1) {
			TreeLeaf* one_leaf = new TreeLeaf();

			one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
			one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
			one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
			one_leaf->bound.edges[1].second = SimRTree::network_maximum_y + SimRTree::division_y_unit_ * to_values_y;
			one_leaf->item_id = parent_->item_id * 2;
			one_leaf->is_leaf = true;
			one_leaf->agent_buffer.clear();

			parent_->items.push_back(one_leaf);

			return;
		}

		//try to cut y
		int where_to_divide_h = -1;
		double cost_to_divide_h = std::numeric_limits<double>::max();
		DivideHorizontal()(from_values_x, from_values_y, to_values_x, to_values_y, &where_to_divide_h, &cost_to_divide_h);

//		std::cout << "where_to_divide_h:" << where_to_divide_h << std::endl;
//		std::cout << "cost_to_divide_h:" << cost_to_divide_h << std::endl;

		//try to cut x
		int where_to_divide_v = -1;
		double cost_to_divide_v = std::numeric_limits<double>::max();
		DivideVertical()(from_values_x, from_values_y, to_values_x, to_values_y, &where_to_divide_v, &cost_to_divide_v);

//		std::cout << "where_to_divide_v:" << where_to_divide_v << std::endl;
//		std::cout << "cost_to_divide_v:" << cost_to_divide_v << std::endl;

		if (cost_to_divide_h >= cost_to_divide_v) {
//			std::cout << "cost_to_divide_h is better:" << where_to_divide_h << std::endl;
			//build leaf
			if (current_level >= target_level) {
				TreeLeaf* one_leaf = new TreeLeaf();

				one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				one_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
				one_leaf->item_id = parent_->item_id * 2;
				one_leaf->is_leaf = true;
				one_leaf->agent_buffer.clear();

				parent_->items.push_back(one_leaf);

				TreeLeaf* second_leaf = new TreeLeaf();

				second_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				second_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				second_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
				second_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				second_leaf->item_id = parent_->item_id * 2 + 1;
				second_leaf->is_leaf = true;
				second_leaf->agent_buffer.clear();

				parent_->items.push_back(second_leaf);
			}
			//build node
			else {
				TreeNode* one_node = new TreeNode();
				one_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				one_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				one_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				one_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
				one_node->item_id = parent_->item_id * 2;
				one_node->is_leaf = false;
				one_node->items.clear();
				parent_->items.push_back(one_node);
				RebuildSimTreeNodeFunctor()(from_values_x, from_values_y, to_values_x, where_to_divide_h, current_level + 1, target_level, one_node);

				TreeNode* second_node = new TreeNode();
				second_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				second_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				second_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * (where_to_divide_h);
				second_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				second_node->item_id = parent_->item_id * 2 + 1;
				second_node->is_leaf = false;
				second_node->items.clear();
				parent_->items.push_back(second_node);
				RebuildSimTreeNodeFunctor()(from_values_x, where_to_divide_h, to_values_x, to_values_y, current_level + 1, target_level, second_node);
			}
		}
		//cut Vertically
		else {
//			std::cout << "cost_to_divide_v is better:" << std::endl;
			//			std::cout << "it is right to go here, but check where_to_divide_v:" << where_to_divide_v << ", and SimRTree::division_x_unit_:" << SimRTree::division_x_unit_ << std::endl;

			//build leaf
			if (current_level >= target_level) {
				TreeLeaf* one_leaf = new TreeLeaf();

				one_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				one_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
				one_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				one_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				one_leaf->item_id = parent_->item_id * 2;
				one_leaf->is_leaf = true;
				one_leaf->agent_buffer.clear();

				parent_->items.push_back(one_leaf);

				TreeLeaf* second_leaf = new TreeLeaf();

				second_leaf->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
				second_leaf->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * to_values_x;
				second_leaf->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				second_leaf->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				second_leaf->item_id = parent_->item_id * 2 + 1;
				second_leaf->is_leaf = true;
				second_leaf->agent_buffer.clear();

				parent_->items.push_back(second_leaf);
			}
			//build node
			else {
				TreeNode* one_node = new TreeNode();
				one_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * from_values_x;
				one_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
				one_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				one_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				one_node->item_id = parent_->item_id * 2;
				one_node->is_leaf = false;
				one_node->items.clear();
				parent_->items.push_back(one_node);
				RebuildSimTreeNodeFunctor()(from_values_x, from_values_y, where_to_divide_v, to_values_y, current_level + 1, target_level, one_node);

				TreeNode* second_node = new TreeNode();
				second_node->bound.edges[0].first = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (where_to_divide_v);
				second_node->bound.edges[0].second = SimRTree::network_minimum_x + SimRTree::division_x_unit_ * (to_values_x);
				second_node->bound.edges[1].first = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * from_values_y;
				second_node->bound.edges[1].second = SimRTree::network_minimum_y + SimRTree::division_y_unit_ * to_values_y;
				second_node->item_id = parent_->item_id * 2 + 1;
				second_node->is_leaf = false;
				second_node->items.clear();
				parent_->items.push_back(second_node);
				RebuildSimTreeNodeFunctor()(where_to_divide_v, from_values_y, to_values_x, to_values_y, current_level + 1, target_level, second_node);
			}
		}
	}
};

#endif

/**
 * Implement
 */

void sim_mob::SimRTree::build_tree_structure(const std::string& filename)
{
	std::ifstream fin(filename.c_str());
	if (!fin) {
		std::cerr << "Cannot open Density Pattern file: " << filename << " " << std::endl;
		return;
	}

	int parent_id, self_id; //IDs used to build the structure
	int is_leaf; //1:YES; 0:NO
	long start_x, start_y, end_x, end_y;

#ifdef USE_REBALANCE
	bool first_line = true;
#endif

	while (fin) {
		fin >> parent_id >> self_id >> start_x >> start_y >> end_x >> end_y >> is_leaf;
		if (!fin.good())
			continue; // skip newlines, etc.

		//the parameters are used in re-balance
#ifdef USE_REBALANCE
		if (first_line) {
			first_line = false;

			SimRTree::network_minimum_x = start_x;
			SimRTree::network_minimum_y = start_y;
			SimRTree::network_maximum_x = end_x;
			SimRTree::network_maximum_y = end_y;

			SimRTree::division_x_unit_ = (int) (ceil((end_x - start_x) * 1.0 / 1000));
			SimRTree::division_y_unit_ = (int) (ceil((end_y - start_y) * 1.0 / 1000));

//			std::cout << " SimRTree::division_x_unit_! " << SimRTree::division_x_unit_ << std::endl;
//			std::cout << " network_minimum_x " << SimRTree::network_minimum_x << std::endl;
//			std::cout << " network_minimum_y " << SimRTree::network_minimum_y << std::endl;
//			std::cout << " network_maximum_x " << SimRTree::network_maximum_x << std::endl;
//			std::cout << " network_maximum_y " << SimRTree::network_maximum_y << std::endl;
		}
#endif

		//used by both node and leaf
		SimRTree::BoundingBox box;
		box.edges[0].first = start_x;
		box.edges[0].second = end_x;
		box.edges[1].first = start_y;
		box.edges[1].second = end_y;

		if (is_leaf == 0) {
			//if the node is root
			if (parent_id == 0) {
				m_root = new TreeNode();
				m_root->is_leaf = false;
				m_root->item_id = self_id;

				m_root->items.reserve(2);
				m_root->bound = box;
			} else {
				TreeNode* one_node = new TreeNode();
				one_node->is_leaf = false;
				one_node->item_id = self_id;

				one_node->items.reserve(2);
				one_node->bound = box;

				addNodeToFather(parent_id, m_root, one_node);
				//				TreeNode* parent = static_cast<TreeNode*> (getNodeByID(parent_id));
				//				parent->items.push_back(one_node);
			}
		}
		//if it is a leaf
		else {
			TreeLeaf* one_leaf = new TreeLeaf();
			one_leaf->next = NULL;

			one_leaf->is_leaf = true;
			one_leaf->item_id = self_id;

			one_leaf->bound = box;

			addNodeToFather(parent_id, m_root, one_leaf);

			//			TreeNode* parent = static_cast<TreeNode*> (getNodeByID(parent_id));
			//			parent->items.push_back(one_leaf);
		}
	}

	fin.close();

	//xuyan:make sure it is done, after the tree is built.
	connectLeaf();

	//xuyan: for re-balance
	countLeaf();

	//temp, should remove
	//	rebalance();
}

void sim_mob::SimRTree::connectLeaf()
{
	connectLeafs(m_root);
	first_leaf = get_leftest_leaf(m_root);
}

/**
 *
 */

void sim_mob::SimRTree::connectLeafs(TreeNode * one_node)
{
	int child_size = one_node->items.size();
	if (child_size <= 1)
		return;

	for (int i = 1; i < child_size; i++) {
		TreeLeaf* right_left = get_rightest_leaf(one_node->items[i - 1]);
		TreeLeaf* left_left = get_leftest_leaf(one_node->items[i]);

		assert(right_left != NULL);
		assert(left_left != NULL);

		right_left->next = left_left;
	}

	for (int i = 0; i < child_size; i++) {
		if (!one_node->items[i]->is_leaf) {
			TreeNode* node = static_cast<TreeNode*>(one_node->items[i]);
			connectLeafs(node);
		}
	}
}

/**
 *
 */
TreeLeaf* sim_mob::SimRTree::get_rightest_leaf(TreeItem* item)
{
	if (!item->is_leaf) {
		TreeNode* node = static_cast<TreeNode*>(item);
		int child_size = node->items.size();
		assert(child_size > 0);

		return get_rightest_leaf(node->items[child_size - 1]);
	} else {
		TreeLeaf* leaf = static_cast<TreeLeaf*>(item);
		return leaf;
	}
}

TreeLeaf* sim_mob::SimRTree::get_leftest_leaf(TreeItem* item)
{
	if (!item->is_leaf) {
		TreeNode* node = static_cast<TreeNode*>(item);
		int child_size = node->items.size();
		assert(child_size > 0);

		return get_leftest_leaf(node->items[0]);
	} else {
		TreeLeaf* leaf = static_cast<TreeLeaf*>(item);
		return leaf;
	}
}

/**
 *not implemented
 */
void sim_mob::SimRTree::releaseTreeMemory()
{
	releaseNodeMemory(m_root);
}

void sim_mob::SimRTree::releaseNodeMemory(TreeItem* item)
{
	if (!item->is_leaf) {
		TreeNode* node = static_cast<TreeNode*>(item);
		for (unsigned int i = 0; i < node->items.size(); i++) {
			releaseNodeMemory(node->items[i]);
		}
	}

	if (item)
		delete item;
}

/**
 *
 */
void sim_mob::SimRTree::countLeaf()
{
	leaf_counts = 0;
	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		leaf_counts++;
		one_leaf = one_leaf->next;
	}
}

/**
 *
 */
void sim_mob::SimRTree::addNodeToFather(std::size_t father_id, TreeNode * from_node, TreeItem* new_node)
{
	if (from_node->is_leaf)
		return;

	if (from_node->item_id == father_id) {
		from_node->items.push_back(new_node);
		return;
	}

	for (unsigned int i = 0; i < from_node->items.size(); i++) {
		if (!from_node->items[i]->is_leaf) {
			TreeNode* one_node = static_cast<TreeNode*>(from_node->items[i]);

			addNodeToFather(father_id, one_node, new_node);
		}
	}
}

/**
 *
 */
SimRTree::BoundingBox sim_mob::SimRTree::location_bounding_box(Agent * agent)
{
	SimRTree::BoundingBox box;
	box.edges[0].first = box.edges[0].second = agent->xPos.get();
	box.edges[1].first = box.edges[1].second = agent->yPos.get();

//	std::cout << "Agent:xPos" << agent->xPos.get();
//	std::cout << ",Agent:yPos" << agent->yPos.get() << std::endl;

	return box;
}

SimRTree::BoundingBox sim_mob::SimRTree::OD_bounding_box(Agent * agent)
{
	SimRTree::BoundingBox box;
	box.edges[0].first = box.edges[0].second = agent->originNode->location.getX();
	box.edges[1].first = box.edges[1].second = agent->originNode->location.getY();

	//	std::cout << "Agent:xPos" << agent->xPos.get();
	//	std::cout << ",Agent:yPos" << agent->yPos.get() << std::endl;

	return box;
}

void sim_mob::SimRTree::insertAgent(Agent* agent)
{
	SimRTree::BoundingBox agent_box = location_bounding_box(agent);
	insertAgentEncloseBox(agent, agent_box, (m_root));
}

void sim_mob::SimRTree::insertAgentBasedOnOD(Agent* agent)
{
	SimRTree::BoundingBox agent_box = OD_bounding_box(agent);
	insertAgentEncloseBox(agent, agent_box, (m_root));
}

void sim_mob::SimRTree::insertAgentEncloseBox(Agent* agent, SimRTree::BoundingBox & agent_box, TreeItem* item)
{
	std::queue<TreeItem*> items;

	items.push(m_root);

	while (!items.empty()) {
		TreeItem* item = items.front();
		items.pop();

		if (item->is_leaf) {
			TreeLeaf* one_leaf = static_cast<TreeLeaf*>(item);

//			std::cout << one_leaf->agent_buffer[0]

			(one_leaf->agent_buffer).push_back(agent);
			return;
		} else {
			TreeNode* node = static_cast<TreeNode*>(item);
			for (unsigned int i = 0; i < node->items.size(); i++) {
				if (node->items[i]->bound.encloses(agent_box)) {
					items.push(node->items[i]);
				}
			}
		}
	}	//end of WHILE LOOP
}

/**
 * Query
 */

std::vector<Agent const*> sim_mob::SimRTree::rangeQuery(SimRTree::BoundingBox & box) const
{
	std::vector<Agent const*> result;

	if (m_root) {
		QueryFunctor query(AcceptEnclosing(box), Collecting_Visitor_sim(result, box));
		query(m_root);
	}

	return result;
}

/**
 *
 */
void sim_mob::SimRTree::updateAllInternalAgents()
{
	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		std::vector<Agent *>& list = one_leaf->agent_buffer;
		SimRTree::BoundingBox box = one_leaf->bound;

		unsigned int offset = 0;
		while (offset < list.size()) {
			//Case 1: the agent should be removed from the Sim-R Tree
			Agent * one_agent = ((list)[offset]);

			if (one_agent->can_remove_by_RTREE) {

//				std::cout << "one_agent->isToBeRemoved():" << one_agent->getId() <<std::endl;
				list.erase(list.begin() + (offset));
				continue;
			}

			SimRTree::BoundingBox agent_box = location_bounding_box((list)[offset]);

			//Case 2: the agent should be in the same box
			//Case 2: It should be the most happen case.
			if (box.encloses(agent_box)) {
				offset++;
				continue;
			}

			//Case 3: The agent should be moved to a different box
			//Case 3: If the agent was insert into a box which is not scanned, the agent will be checked again. It will cost some CPU. But, since Case 3 is not much, the cost should be accpatable.
			//Case 3: If the cost is heavy, need to define another variable in agent to skip re-check.
			else {
//				int size_b = (int) list.size();
//				insertAgent((list)[offset]);
//
//				int size_i = (int) list.size();
//				list.erase(list.begin() + (offset));
//
//				int size_e = (int) list.size();
//
//				if(size_i != size_b)
//				{
//					std::cout << "Wired: " << size_i << "," << size_b << "," << size_e <<std::endl;
//				}

				Agent * one_agent = (list)[offset];
				list.erase(list.begin() + (offset));
				insertAgent(one_agent);

			}
		} // end of inside while

		one_leaf = one_leaf->next;
	} // end of out while loop
}
/**
 *
 */

void sim_mob::SimRTree::display()
{
	std::cout << "#########################################" << std::endl;
	display(m_root, 1);
}

void sim_mob::SimRTree::display(TreeItem* item, int level)
{
	if (!item->is_leaf) {
		std::cout << "level:" << level << "(" << item->bound.edges[0].first << "," << item->bound.edges[1].first << "," << item->bound.edges[0].second << "," << item->bound.edges[1].second << ")"
				<< std::endl;

		TreeNode* one_node = static_cast<TreeNode*>(item);
		int child_size = one_node->items.size();
		for (int i = 0; i < child_size; i++) {
			display((one_node->items[i]), level + 1);
		}
	} else {
		TreeLeaf* one_leaf = static_cast<TreeLeaf*>(item);
		std::cout << "level:" << level << "(" << item->bound.edges[0].first << "," << item->bound.edges[1].first << "," << item->bound.edges[0].second << "," << item->bound.edges[1].second << ")"
				<< "child size" << one_leaf->agent_buffer.size() << std::endl;

		//		std::cout << "--------------------" << std::endl;
		//
		//		for (unsigned int i = 0; i < one_leaf->agent_buffer.size(); i++)
		//		{
		//			std::cout << i << ":" << one_leaf->agent_buffer[i]->loc_x << "," << one_leaf->agent_buffer[i]->loc_y << std::endl;
		//		}
		//
		//		std::cout << "--------------------" << std::endl;
	}
}

/**
 * Debug
 */
void sim_mob::SimRTree::checkLeaf()
{
	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		int agents_size = one_leaf->agent_buffer.size();

//		std::cout << "agents_size:" << agents_size << std::endl;

		for (int i = 0; i < agents_size; i++) {
			if (one_leaf->agent_buffer[i]->isToBeRemoved()) {
				std::cout << "one_leaf->agent_buffer[i]->isToBeRemoved():" << one_leaf->agent_buffer[i]->getId() << std::endl;
			}
		}

		one_leaf = one_leaf->next;
	}
}

/**
 * Load Balance
 */
void sim_mob::SimRTree::measureUnbalance(int time_step)
{
#ifdef USE_REBALANCE
	//check every 3 time step
	//hard coded
	if (time_step % checking_frequency != 0)
	return;

	/**
	 * There is another measurement names: load ratio
	 */

	double total_cost = 0;
	double best_cost = 0;
	int depth_cost = 20;

	std::vector<int> agents_counts_in_tree;

	leaf_agents_sum = 0;
	unbalance_ratio = 0;

	TreeLeaf* one_leaf = first_leaf;

	while (one_leaf) {
		int agents_size = one_leaf->agent_buffer.size();

		total_cost += agents_size * (agents_size + depth_cost);

		agents_counts_in_tree.push_back(agents_size);
		leaf_agents_sum += agents_size;

		one_leaf = one_leaf->next;
	}

	if (leaf_agents_sum <= 0)
	return;

	double average_counts = leaf_agents_sum / leaf_counts;
	best_cost = average_counts * (average_counts + depth_cost) * leaf_counts;

	//calculate un-balance ratio
	double temp_value_for_unbalance_ratio = 0;
	for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
		temp_value_for_unbalance_ratio += pow((*it - average_counts), 2);
	}

	temp_value_for_unbalance_ratio = temp_value_for_unbalance_ratio * leaf_counts;
	temp_value_for_unbalance_ratio = sqrt(temp_value_for_unbalance_ratio);
	temp_value_for_unbalance_ratio = temp_value_for_unbalance_ratio / leaf_agents_sum;

	unbalance_ratio = temp_value_for_unbalance_ratio;

//	std::cout << "average_counts:" << average_counts << std::endl;
//	std::cout << "leaf_agents_sum:" << leaf_agents_sum << std::endl;
//	std::cout << "leaf_counts:" << leaf_counts << std::endl;
//	std::cout << "unbalance_ratio:" << unbalance_ratio << std::endl;
//	std::cout << "total load cost:" << total_cost << std::endl;
//	std::cout << "best load cost:" << best_cost << std::endl;
//	std::cout << "average_leaf_counts:" << average_counts << std::endl;
//	std::cout << "load ratio:" << total_cost / best_cost << std::endl;
//
//	std::cout << "===========================" << std::endl;

	//	//need to re-build the tree
	if (average_counts > 32 || average_counts < 1 || (total_cost / best_cost) > 1.2) {
		rebalance_counts++;
		//		std::cout << "average_counts:" << average_counts << std::endl;
		//		std::cout << "total_cost / best_cost:" << total_cost / best_cost << std::endl;

		//check every 2 times
		//hard coded

		if (rebalance_counts >= rebalance_threshold) {
			//			std::cout << "1->2" << std::endl;
			rebalance_counts = 0;

//			for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
//				std::cout << (*it) << ",	";
//			}
//			std::cout << std::endl;

			rebalance();
		}
	} else {
		//		std::cout << "1->0" << std::endl;
		rebalance_counts = 0;
	}

	//temp always call re-balance

	//	display();

	//	rebalance();
#endif
}

void sim_mob::SimRTree::rebalance()
{
//	std::cout << "rebalance:" << std::endl;

#ifdef USE_REBALANCE
	//release memory
	releaseTreeMemory();

	//Build A Big Table
	memset(bigtable, 0, sizeof(bigtable));

	//set the agent location
	//use structure & loop
	for_each(Agent::all_agents.begin(), Agent::all_agents.end(), BigTableUpdate());

//	std::cout << std::endl;
//	for (int i = 0; i < 1000; i++) {
//		for (int j = 0; j < 1000; j++) {
//
//			if (SimRTree::bigtable[i][j] > 0)
//				std::cout << SimRTree::bigtable[i][j] << ",";
//		}
//	}
//	std::cout << std::endl;

	// 11 signals
	int agent_size = Agent::all_agents.size() - 11;
	int tree_height = 1;

	//16 is temp setting
	while (agent_size > 16) {
		agent_size = agent_size / 2;
		tree_height++;

//		std::cout << "tree_height:" << tree_height << ",agent_size:" << agent_size << std::endl;
	}

	//build new tree & set root
	TreeNode* one_node = new TreeNode();
	one_node->bound.edges[0].first = SimRTree::network_minimum_x;
	one_node->bound.edges[0].second = SimRTree::network_maximum_x;
	one_node->bound.edges[1].first = SimRTree::network_minimum_y;
	one_node->bound.edges[1].second = SimRTree::network_maximum_y;
	one_node->item_id = 0;
	one_node->is_leaf = false;
	one_node->items.clear();

	//the division is hard coded inside
	RebuildSimTreeNodeFunctor()(0, 0, 1000, 1000, 1, tree_height, one_node);

	//	rebuildSimTree(one_node, 1, tree_height, 0, 0, 10000, 1);
	m_root = one_node;

	//xuyan:make sure it is done, after the tree is built.
	connectLeaf();

	//xuyan: for re-balance
	countLeaf();

//		std::cout << "Agent::all_agents:" << Agent::all_agents.size() << std::endl;

	//re-insert all agents inside
	for (std::vector<Entity*>::iterator itr = Agent::all_agents.begin(); itr != Agent::all_agents.end(); itr++) {
		Agent* agent = dynamic_cast<Agent*>(*itr);
		if (agent && (agent)->isToBeRemoved() == false)
		insertAgent(agent);
	}

//	std::cout << "-----------------------------------------------" << std::endl;
//	display();
	//temp

//	double total_cost = 0;
//	double best_cost = 0;
//	int depth_cost = 20;
//
//	std::vector<int> agents_counts_in_tree;
//
//	leaf_agents_sum = 0;
//	unbalance_ratio = 0;
//
//	TreeLeaf* one_leaf = first_leaf;
//
//	while (one_leaf) {
//		int agents_size = one_leaf->agent_buffer.size();
//
//		total_cost += agents_size * (agents_size + depth_cost);
//
//		agents_counts_in_tree.push_back(agents_size);
//		leaf_agents_sum += agents_size;
//
//		one_leaf = one_leaf->next;
//	}
//
//	if (leaf_agents_sum <= 0)
//		return;
//
//	double average_counts = leaf_agents_sum / leaf_counts;
//	best_cost = average_counts * (average_counts + depth_cost) * leaf_counts;
//
//	//calculate un-balance ratio
//	double temp_value_for_unbalance_ratio = 0;
//	for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
//		temp_value_for_unbalance_ratio += pow((*it - average_counts), 2);
//	}
//
//	temp_value_for_unbalance_ratio = temp_value_for_unbalance_ratio * leaf_counts;
//	temp_value_for_unbalance_ratio = sqrt(temp_value_for_unbalance_ratio);
//	temp_value_for_unbalance_ratio = temp_value_for_unbalance_ratio / leaf_agents_sum;
//
//	unbalance_ratio = temp_value_for_unbalance_ratio;
//
//	std::cout << "After-------------------------------" << std::endl;
//
//	for (std::vector<int>::iterator it = agents_counts_in_tree.begin(); it != agents_counts_in_tree.end(); it++) {
//		std::cout << (*it) << ",	";
//	}
//	std::cout << std::endl;

//use re-balance
#endif

}
