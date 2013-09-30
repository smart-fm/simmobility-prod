/*
 *  Copyright (c) 2008 Dustin Spicuzza <dustin@virtualroadside.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of version 2 of the GNU General Public License
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 *	This is intended to be a templated implementation of an R* Tree, designed
 *	to create an efficient and (relatively) small indexing container in N 
 *	dimensions. At the moment, it is a memory-based container instead of disk
 *  based.
 *
 *	Based on "The R*-Tree: An Efficient and Robust Access Method for Points 
 *	and Rectangles" by N. Beckmann, H.P. Kriegel, R. Schneider, and B. Seeger
 */

#pragma once

//#define USE_R_DU_TREE
//#ifdef USE_R_DU_TREE
#include "spatial_trees/spatial_tree_include.hpp"

#include <map>
#include <list>
#include <vector>
#include <limits>
#include <algorithm>
#include <cassert>
#include <functional>

#include <iostream>
#include <sstream>
#include <fstream>

#include "spatial_trees/rstar_tree/RStarBoundingBox.hpp"
//#include "spatial_trees/Debug_Static_Count.hpp"

namespace { //R* tree parameters, unique to this translational unit.
const double RDUTREE_REINSERT_P = 0.30;
const size_t RDUTREE_CHOOSE_SUBTREE_P = 32;
}

namespace {
class Profiling_Counting_RDU{
public:
	static long counting;
	static std::vector<long> count_buff;
};

long Profiling_Counting_RDU::counting = 0;
std::vector<long> Profiling_Counting_RDU::count_buff;
}

// definition of a node
template<typename BoundedItem>
struct RStarDUNode: BoundedItem {
	std::vector<BoundedItem*> items;
	bool hasLeaves;
};

// definition of an leaf
template<typename BoundedItem, typename LeafType>
struct RStarDULeaf: BoundedItem {
	typedef LeafType leaf_type;
	LeafType leaf;
	BoundedItem* father;
};

#include "spatial_trees/rstar_tree/RStarVisitor.hpp"

/**
 \class RStarTreeDownUp
 \brief Implementation of an RTree with an R* index

 @tparam LeafType		type of leaves stored in the tree
 @tparam dimensions  	number of dimensions the bounding boxes are described in
 @tparam	min_child_items m, in the range 2 <= m < M
 @tparam max_child_items M, in the range 2 <= m < M
 @tparam	RemoveLeaf 		A functor used to remove leaves from the tree
 */
template<typename LeafType, std::size_t dimensions, std::size_t min_child_items, std::size_t max_child_items>
class RStarTreeDownUp {
public:

	// shortcuts
	typedef RStarBoundedItem<dimensions> BoundedItem;
	typedef typename BoundedItem::BoundingBox BoundingBox;

	typedef RStarDUNode<BoundedItem> Node;
	typedef RStarDULeaf<BoundedItem, LeafType> Leaf;

	// acceptors
	typedef RStarAcceptOverlapping<Node, Leaf> AcceptOverlapping;
	typedef RStarAcceptEnclosing<Node, Leaf> AcceptEnclosing;
	typedef RStarAcceptAny<Node, Leaf> AcceptAny;

	// predefined visitors
	typedef RStarRemoveLeaf<Leaf> RemoveLeaf;
	typedef RStarRemoveSpecificLeaf<Leaf> RemoveSpecificLeaf;

	// default constructor
	RStarTreeDownUp() :
			m_root(NULL), m_size(0) {
		assert(1 <= min_child_items && min_child_items <= max_child_items/2);
	}

	// destructor
	~RStarTreeDownUp() {
		Remove(AcceptAny(), RemoveLeaf());
		second_index.clear();
	}

	void DEBUG_(Node* node, int index) {
		if (node->hasLeaves) {
			std::cout << index << ",node.items" << node->items.size();
			std::cout << std::endl;
		} else {
			for (unsigned int i = 0; i < node->items.size(); i++) {
				Node* one = static_cast<Node*>(node->items[i]);
				if (one)
					DEBUG_(one, index + 1);
			}
		}
	}

	void Check_() {
		std::cout << "-------------------------------------------------" << std::endl;
		Check_(m_root, 1);
	}

	void Check_(Node* node, int index) {
		for (unsigned int i = 0; i < node->items.size(); i++) {
			if (!node->bound.encloses(node->items[i]->bound)) {
				std::cout << " Error " << std::endl;
			}
		}

		for (unsigned int i = 0; i < node->items.size(); i++) {
			if (!node->items[i]->is_a_leaf) {
				Node* one = static_cast<Node*>(node->items[i]);
				Check_(one, index + 1);
			}
		}
	}

	//update new location
	void update_(int object_id, int new_location_x, int new_location_y) {

		typename std::map<int, Leaf*>::iterator it = second_index.find(object_id);
		if (it != second_index.end()) {
			BoundingBox box;
			box.edges[0].first = new_location_x;
			box.edges[1].first = new_location_y;
			box.edges[0].second = new_location_x;
			box.edges[1].second = new_location_y;

			Leaf* the_leaf = it->second;
			if (the_leaf->father->bound.encloses(box)) {
				//do nothing
				the_leaf->bound.edges[0].first = new_location_x;
				the_leaf->bound.edges[0].second = new_location_x;
				the_leaf->bound.edges[1].first = new_location_y;
				the_leaf->bound.edges[1].second = new_location_y;
			} else {
				//remove & re-insert
				second_index.erase(object_id);
				Remove(AcceptAny(), RemoveSpecificLeaf(the_leaf->leaf));

				//
				Insert(the_leaf->leaf, box, object_id);

				it = second_index.find(object_id);
				assert(it != second_index.end());
			}
		} else {
			std::cout << "Error: agent is not exsiting when update" << std::endl;
		}
	}

	//remove
	void remove_(int object_id) {
		typename std::map<int, Leaf*>::iterator it = second_index.find(object_id);
		if (it != second_index.end()) {
			Leaf* the_leaf = it->second;
			//			Remove(AcceptAny(), RemoveSpecificLeaf(the_leaf->leaf));

			second_index.erase(object_id);
			Remove(AcceptAny(), RemoveSpecificLeaf(the_leaf->leaf));

//			RemoveOne(AcceptAny(), RemoveSpecificLeaf(the_leaf->leaf), object_id);
		} else {
			std::cout << "Error: agent is not exsiting when remove" << std::endl;
		}
	}

	bool has_one_object(int object_id) {
		typename std::map<int, Leaf*>::iterator it = second_index.find(object_id);
//		if (it != second_index.end())
//		{
//			Leaf* leaf = it->second;
//			std::cout << "in DU Tree Father:" << leaf->father->bound.edges[0].first;
//			std::cout << ":" << leaf->father->bound.edges[0].second;
//			std::cout << ":" << leaf->father->bound.edges[1].first;
//			std::cout << ":" << leaf->father->bound.edges[1].second << std::endl;
//			std::cout << "in DU Tree Itself:" << leaf->bound.edges[0].first;
//			std::cout << ":" << leaf->bound.edges[0].second;
//			std::cout << ":" << leaf->bound.edges[1].first;
//			std::cout << ":" << leaf->bound.edges[1].second << std::endl;
//		}

		return (it != second_index.end());
	}

	// Single insert function, adds a new item to the tree
	void Insert(LeafType leaf, const BoundingBox &bound, int object_id) {
#ifdef MEAUSURE_COUNTS
		static long Insert_counts = 0;
		Insert_counts++;
		std::cout << "Insert_counts:" << Insert_counts << std::endl;
#endif

//		std::cout << "Insert 0.1" << std::endl;

		// ID1: Invoke Insert starting with the leaf level as a
		// parameter, to Insert a new data rectangle
		Leaf * newLeaf = new Leaf();
		newLeaf->bound = bound;
		newLeaf->leaf = leaf;
		newLeaf->is_a_leaf = true;

//		std::cout << "Agent:" << bound.edges[0].first << "," << bound.edges[1].first << std::endl;

		second_index.insert(std::make_pair(object_id, newLeaf));

		// create a new root node if necessary
		if (!m_root) {
			m_root = new Node();
			m_root->hasLeaves = true;

			//xuyan
			newLeaf->father = m_root;

			// reserve memory
			m_root->items.reserve(min_child_items);
			m_root->items.push_back(newLeaf);
			m_root->bound = bound;
			m_root->is_a_leaf = false;
		} else
			// start the insertion process
			InsertInternal(newLeaf, m_root);

		m_size += 1;
	}

	/**
	 \brief Touches each node using the visitor pattern

	 You must specify an	acceptor functor that takes a BoundingBox and a
	 visitor that takes a BoundingBox and a const LeafType&.

	 See RStarVisitor.h for more information about the various visitor
	 types available.

	 @param acceptor 		An acceptor functor that returns true if this
	 branch or leaf of the tree should be considered for visitation.

	 @param visitor			A visitor functor that does the visiting

	 @return This will return the Visitor object, so you can retrieve whatever
	 data it has in it if needed (for example, to get the count of items
	 visited). It returns by value, so ensure that the copy is cheap
	 for decent performance.
	 */
	template<typename Acceptor, typename Visitor>
	Visitor Query(const Acceptor &accept, Visitor visitor) const {

//		sim_mob::Debug_Static_Count::query_depth = 0;
//		std::cout << "=========================" << std::endl;
//		std::cout << "start" << std::endl;

#ifdef QUERY_PROFILING
		Profiling_Counting_RDU::counting = 0;
#endif


		if (m_root) {
			QueryFunctor<Acceptor, Visitor> query(accept, visitor);
			query(m_root);
		}

#ifdef QUERY_PROFILING
		long one = Profiling_Counting_RDU::counting;
		Profiling_Counting_RDU::count_buff.push_back(one);

		if(Profiling_Counting_RDU::count_buff.size() % 1000000 == 0)
		{
			double sum = 0;

			for(int i=0;i<Profiling_Counting_RDU::count_buff.size();i++)
			{
				sum += Profiling_Counting_RDU::count_buff[i];
			}

			std::cout << "Count:" << Profiling_Counting_RDU::count_buff.size() << ", Sum Cost:" << sum << std::endl;
		}

//		std::cout << "Profiling_Counting_RDU::counting:" << Profiling_Counting_RDU::counting << std::endl;
#endif

//		std::cout << "debug_count" << sim_mob::Debug_Static_Count::query_depth << std::endl;
//		std::cout << "end" << std::endl;
//		std::cout << "=========================" << std::endl;

		return visitor;
	}

	template<typename Acceptor, typename Visitor>
	Visitor DebugQuery(const Acceptor &accept, Visitor visitor) const {
		if (m_root) {
			DebugStructureFunctor<Acceptor, Visitor> query(accept, visitor);
			query(m_root);
		}

		return visitor;
	}

	/**
	 \brief Removes item(s) from the tree.

	 See RStarVisitor.h for more information about the various visitor
	 types available.

	 @param acceptor 	A node acceptor functor that returns true if this
	 branch or leaf of the tree should be considered for deletion
	 (it does not delete it, however. That is what the LeafRemover does).

	 @param leafRemover		A visitor functor that decides whether that
	 individual item should be removed from the tree. If it returns true,
	 then the node holding that item will be deleted.

	 See also RemoveBoundedArea, RemoveItem for examples of how this
	 function can be called.
	 */
	template<typename Acceptor, typename LeafRemover>
	void Remove(const Acceptor &accept, LeafRemover leafRemover) {
#ifdef MEAUSURE_COUNTS
		static long Remove_counts = 0;
		Remove_counts++;
		std::cout << "Remove_counts:" << Remove_counts << std::endl;
#endif
		std::list<Leaf*> itemsToReinsert;

		if (!m_root)
			return;

		RemoveFunctor<Acceptor, LeafRemover> remove(accept, leafRemover, &itemsToReinsert, &m_size);
		remove(m_root, true);

//		std::cout << "remove 1.1" << std::endl;

		if (!itemsToReinsert.empty()) {

#ifdef MEAUSURE_COUNTS
		static long Merge_counts = 0;
		Merge_counts++;
		std::cout << "Merge_counts:" << Merge_counts << std::endl;
#endif
//			std::cout << "remove 1.2" << std::endl;

			// reinsert anything that needs to be reinserted
			typename std::list<Leaf*>::iterator it = itemsToReinsert.begin();
			typename std::list<Leaf*>::iterator end = itemsToReinsert.end();

			// TODO: do this whenever that actually works..
			// BulkInsert(itemsToReinsert, m_root);

			for (; it != end; it++)
				InsertInternal(*it, m_root);
		}
	}

//	template<typename Acceptor, typename LeafRemover>
//	void RemoveOne(const Acceptor &accept, LeafRemover leafRemover, int object_id)
//	{
//		std::list<Leaf*> itemsToReinsert;
//
//		if (!m_root)
//			return;
//
//		second_index.erase(object_id);
//
//		RemoveFunctor<Acceptor, LeafRemover> remove(accept, leafRemover, &itemsToReinsert, &m_size);
//		remove(m_root, true);
//
//		if (!itemsToReinsert.empty()) {
//			// reinsert anything that needs to be reinserted
//			typename std::list<Leaf*>::iterator it = itemsToReinsert.begin();
//			typename std::list<Leaf*>::iterator end = itemsToReinsert.end();
//
//			// TODO: do this whenever that actually works..
//			// BulkInsert(itemsToReinsert, m_root);
//
//			for (; it != end; it++)
//				InsertInternal(*it, m_root);
//		}
//	}

	// stub that removes any items contained in an specified area
	void RemoveBoundedArea(const BoundingBox &bound) {
		Remove(AcceptEnclosing(bound), RemoveLeaf());
	}

	// removes a specific item. If removeDuplicates is true, only the first
	// item found will be removed
	void RemoveItem(const LeafType &item, bool removeDuplicates = true) {
		Remove(AcceptAny(), RemoveSpecificLeaf(item, removeDuplicates));
	}

	std::size_t GetSize() const {
		return m_size;
	}
	std::size_t GetDimensions() const {
		return dimensions;
	}

protected:

	// choose subtree: only pass this items that do not have leaves
	// I took out the loop portion of this algorithm, so it only
	// picks a subtree at that particular level
	Node * ChooseSubtree(Node * node, const BoundingBox * bound) {
		// If the child pointers in N point to leaves
		if (static_cast<Node*>(node->items[0])->hasLeaves) {
			// determine the minimum overlap cost
			if (max_child_items > (RDUTREE_CHOOSE_SUBTREE_P * 2) / 3 && node->items.size() > RDUTREE_CHOOSE_SUBTREE_P) {
//				std::cout << "Case A" << std::endl;

				// ** alternative algorithm:
				// Sort the rectangles in N in increasing order of
				// then area enlargement needed to include the new
				// data rectangle

				// Let A be the group of the first p entrles
				std::partial_sort(node->items.begin(), node->items.begin() + RDUTREE_CHOOSE_SUBTREE_P, node->items.end(), SortBoundedItemsByAreaEnlargement<BoundedItem>(bound));

				// From the items in A, considering all items in
				// N, choose the leaf whose rectangle needs least
				// overlap enlargement

				return static_cast<Node*>(*std::min_element(node->items.begin(), node->items.begin() + RDUTREE_CHOOSE_SUBTREE_P, SortBoundedItemsByOverlapEnlargement<BoundedItem>(bound)));
			}

//			std::cout << "Case B" << std::endl;
//			std::cout << "node->items:" << node->items.size() << std::endl;

			// choose the leaf in N whose rectangle needs least
			// overlap enlargement to include the new data
			// rectangle Resolve ties by choosmg the leaf
			// whose rectangle needs least area enlargement, then
			// the leaf with the rectangle of smallest area

			return static_cast<Node*>(*std::min_element(node->items.begin(), node->items.end(), SortBoundedItemsByOverlapEnlargement<BoundedItem>(bound)));
		}

		// if the chlld pointers in N do not point to leaves

		// [determine the minimum area cost],
		// choose the leaf in N whose rectangle needs least
		// area enlargement to include the new data
		// rectangle. Resolve ties by choosing the leaf
		// with the rectangle of smallest area

//		std::cout << "Case C" << std::endl;
		return static_cast<Node*>(*std::min_element(node->items.begin(), node->items.end(), SortBoundedItemsByAreaEnlargement<BoundedItem>(bound)));
	}

	// inserts nodes recursively. As an optimization, the algorithm steps are
	// way out of order. :) If this returns something, then that item should
	// be added to the caller's level of the tree
	Node * InsertInternal(Leaf * leaf, Node * node, bool firstInsert = true) {
		// I4: Adjust all covering rectangles in the insertion path
		// such that they are minimum bounding boxes
		// enclosing the children rectangles
//		std::cout << "InsertInternal 1.3" << std::endl;

		node->bound.stretch(leaf->bound);

//		std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
//		std::cout << "leaf->bound:" << leaf->bound.edges[0].first << std::endl;
//		std::cout << "leaf->bound:" << leaf->bound.edges[0].second << std::endl;
//		std::cout << "leaf->bound:" << leaf->bound.edges[1].first << std::endl;
//		std::cout << "leaf->bound:" << leaf->bound.edges[1].second << std::endl;
//		std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;

		// CS2: If we're at a leaf, then use that level
		if (node->hasLeaves) {
			// I2: If N has less than M items, accommodate E in N
			node->items.push_back(leaf);
			leaf->father = node;
//			std::cout << "InsertInternal 1.35" << std::endl;
		} else {
			// I1: Invoke ChooseSubtree. with the level as a parameter,
			// to find an appropriate node N, m which to place the
			// new leaf E

			// of course, this already does all of that recursively. we just need to
			// determine whether we need to split the overflow or not
			Node * tmp_node = InsertInternal(leaf, ChooseSubtree(node, &leaf->bound), firstInsert);

			if (!tmp_node)
			{
//				std::cout << "InsertInternal 1.4" << std::endl;
				return NULL;
			}

			// this gets joined to the list of items at this level
			node->items.push_back(tmp_node);
			tmp_node->is_a_leaf = false;

			//			std::cout << "After Split: node->items.size():" << node->items.size() << std::endl;
		}

		// If N has M+1 items. invoke OverflowTreatment with the
		// level of N as a parameter [for reinsertion or split]
		//		std::cout << "======333=========:" << std::endl;
		//
		//		if (node == m_root)
		//		{
		//			std::cout << "ROOT" << std::endl;
		//		}
		//
		//		std::cout << "node->items.size():" << node->items.size() << std::endl;
		//		std::cout << "===================:" << std::endl;

		if (node->items.size() > max_child_items) {
			// I3: If OverflowTreatment was called and a split was
			// performed, propagate OverflowTreatment upwards
			// if necessary

//			std::cout << "InsertInternal 1.5" << std::endl;

			// This is implicit, the rest of the algorithm takes place in there
			return OverflowTreatment(node, firstInsert);
		}

//		std::cout << "InsertInternal 1.6" << std::endl;

		return NULL;
	}

	// TODO: probably could just merge this in with InsertInternal()
	Node * OverflowTreatment(Node * level, bool firstInsert) {
#ifdef MEAUSURE_COUNTS
		static long OverflowTreatment_counts = 0;
		OverflowTreatment_counts++;
		std::cout << "OverflowTreatment:" << OverflowTreatment_counts << std::endl;
#endif
		// OT1: If the level is not the root level AND this is the first
		// call of OverflowTreatment in the given level during the
		// insertion of one data rectangle, then invoke Reinsert
//		std::cout << "OverflowTreatment 1.4" << std::endl;

		if (level != m_root && firstInsert) {
			Reinsert(level);
			return NULL;
		}

		//		DEBUG_(level, 1);
		//		std::cout << "----------------------------" << std::endl;

		Node * splitItem = Split(level);
		//		std::cout << "splitItem:" << splitItem->hasLeaves << std::endl;

		//		DEBUG_(splitItem, 1);

		//		if(splitItem->is_a_leaf)
		//		{
		//			std::cout << "AAAAAAAAAAAA" << std::endl;
		//		}

		//xuyan: set father
		if (splitItem->items[0]->is_a_leaf) {
			//			std::cout << "BVBBBBBBBBBB" << std::endl;

			for (unsigned int i = 0; i < splitItem->items.size(); i++) {
				//				std::cout << "CCCCCCCCCCCCCCC" << std::endl;

				Leaf* a_leaf = static_cast<Leaf*>(splitItem->items[i]);
				//				std::cout << "a_leaf->leaf->agent_id:" << a_leaf->leaf->agent_id << std::endl;
				a_leaf->father = splitItem;
			}
		}

		// If OverflowTreatment caused a split of the root, create a new root
		if (level == m_root) {
			//			DEBUG_(m_root, 1);
			//			std::cout << "----------------------------" << std::endl;
			//			DEBUG_(splitItem, 1);

			//			std::cout << "Split Root:" << std::endl;

			Node * newRoot = new Node();
			newRoot->hasLeaves = false;

			// reserve memory
			newRoot->items.reserve(min_child_items);
			newRoot->items.push_back(m_root);
			newRoot->items.push_back(splitItem);
			newRoot->is_a_leaf = false;

			// Do I4 here for the new root item
			newRoot->bound.reset();
			std::for_each(newRoot->items.begin(), newRoot->items.end(), StretchBoundingBox<BoundedItem>(&newRoot->bound));

			// and we're done
			m_root = newRoot;

			//			DEBUG_(m_root, 1);
			return NULL;
		}

		// propagate it upwards
		return splitItem;
	}

	// this combines Split, ChooseSplitAxis, and ChooseSplitIndex into
	// one function as an optimization (they all share data structures,
	// so it would be pointless to do all of that copying)
	//
	// This returns a node, which should be added to the items of the
	// passed node's parent
	Node * Split(Node * node) {
#ifdef MEAUSURE_COUNTS
		static long Split_counts = 0;
		Split_counts++;
		std::cout << "Split:" << Split_counts << std::endl;
#endif
		Node * newNode = new Node();
		newNode->hasLeaves = node->hasLeaves;

		const std::size_t n_items = node->items.size();
		const std::size_t distribution_count = n_items - 2 * min_child_items + 1;

		std::size_t split_axis = dimensions + 1, split_edge = 0, split_index = 0;
		int split_margin = 0;

		BoundingBox R1, R2;

		//		std::cout << "======Split========:" << std::endl;
		//		std::cout << "n_items:" << n_items << std::endl;
		//		std::cout << "max_child_items:" << max_child_items << std::endl;

		// these should always hold true
		assert(n_items == max_child_items + 1);
		assert(distribution_count > 0);
		assert(min_child_items + distribution_count-1 <= n_items);

		// S1: Invoke ChooseSplitAxis to determine the axis,
		// perpendicular to which the split 1s performed
		// S2: Invoke ChooseSplitIndex to determine the best
		// distribution into two groups along that axis

		// NOTE: We don't compare against node->bound, so it gets overwritten
		// at the end of the loop

		// CSA1: For each axis
		for (std::size_t axis = 0; axis < dimensions; axis++) {
			// initialize per-loop items
			int margin = 0;
			double overlap = 0, dist_area, dist_overlap;
			std::size_t dist_edge = 0, dist_index = 0;

			dist_area = dist_overlap = std::numeric_limits<double>::max();

			// Sort the items by the lower then by the upper
			// edge of their bounding box on this particular axis and
			// determine all distributions as described . Compute S. the
			// sum of all margin-values of the different
			// distributions

			// lower edge == 0, upper edge = 1
			for (std::size_t edge = 0; edge < 2; edge++) {
				// sort the items by the correct key (upper edge, lower edge)
				if (edge == 0)
					std::sort(node->items.begin(), node->items.end(), SortBoundedItemsByFirstEdge<BoundedItem>(axis));
				else
					std::sort(node->items.begin(), node->items.end(), SortBoundedItemsBySecondEdge<BoundedItem>(axis));

				// Distributions: pick a point m in the middle of the thing, call the left
				// R1 and the right R2. Calculate the bounding box of R1 and R2, then
				// calculate the margins. Then do it again for some more points
				for (std::size_t k = 0; k < distribution_count; k++) {
					double area = 0;

					// calculate bounding box of R1
					R1.reset();
					std::for_each(node->items.begin(), node->items.begin() + (min_child_items + k), StretchBoundingBox<BoundedItem>(&R1));

					// then do the same for R2
					R2.reset();
					std::for_each(node->items.begin() + (min_child_items + k + 1), node->items.end(), StretchBoundingBox<BoundedItem>(&R2));

					// calculate the three values
					margin += R1.edgeDeltas() + R2.edgeDeltas();
					area += R1.area() + R2.area(); // TODO: need to subtract.. overlap?
					overlap = R1.overlap(R2);

					// CSI1: Along the split axis, choose the distribution with the
					// minimum overlap-value. Resolve ties by choosing the distribution
					// with minimum area-value.
					if (overlap < dist_overlap || (overlap == dist_overlap && area < dist_area)) {
						// if so, store the parameters that allow us to recreate it at the end
						dist_edge = edge;
						dist_index = min_child_items + k;
						dist_overlap = overlap;
						dist_area = area;
					}
				}
			}

			// CSA2: Choose the axis with the minimum S as split axis
			if (split_axis == dimensions + 1 || split_margin > margin) {
				split_axis = axis;
				split_margin = margin;
				split_edge = dist_edge;
				split_index = dist_index;
			}
		}

		// S3: Distribute the items into two groups

		// ok, we're done, and the best distribution on the selected split
		// axis has been recorded, so we just have to recreate it and
		// return the correct index

		if (split_edge == 0)
			std::sort(node->items.begin(), node->items.end(), SortBoundedItemsByFirstEdge<BoundedItem>(split_axis));

		// only reinsert the sort key if we have to

		else if (split_axis != dimensions - 1)
			std::sort(node->items.begin(), node->items.end(), SortBoundedItemsBySecondEdge<BoundedItem>(split_axis));

		// distribute the end of the array to the new node, then erase them from the original node
		newNode->items.assign(node->items.begin() + split_index, node->items.end());
		node->items.erase(node->items.begin() + split_index, node->items.end());

		// adjust the bounding box for each 'new' node
		node->bound.reset();
		std::for_each(node->items.begin(), node->items.end(), StretchBoundingBox<BoundedItem>(&node->bound));

		newNode->bound.reset();
		std::for_each(newNode->items.begin(), newNode->items.end(), StretchBoundingBox<BoundedItem>(&newNode->bound));

		//		std::cout << "After split node:" << node->items.size() << std::endl;
		//		std::cout << "After split newNode:" << newNode->items.size() << std::endl;
		//		std::cout << "===================:" << std::endl;

		return newNode;
	}

	// This routine is used to do the opportunistic reinsertion that the
	// R* algorithm calls for
	void Reinsert(Node * node) {
#ifdef MEAUSURE_COUNTS
		static long Reinsert_counts = 0;
		Reinsert_counts++;
		std::cout << "Reinsert:" << Reinsert_counts << std::endl;
#endif
		std::vector<BoundedItem*> removed_items;

		const std::size_t n_items = node->items.size();
		const std::size_t p = (std::size_t) ((double) n_items * RDUTREE_REINSERT_P) > 0 ? (std::size_t) ((double) n_items * RDUTREE_REINSERT_P) : 1;

		// RI1 For all M+l items of a node N, compute the distance
		// between the centers of their rectangles and the center
		// of the bounding rectangle of N

		//		std::cout << "========2222=======:" << std::endl;
		//		std::cout << "n_items:" << n_items << std::endl;
		//		std::cout << "max_child_items:" << max_child_items << std::endl;
		//		std::cout << "===================:" << std::endl;
		assert(n_items == max_child_items + 1);

		// RI2: Sort the items in increasing order of their distances
		// computed in RI1
		std::partial_sort(node->items.begin(), node->items.end() - p, node->items.end(), SortBoundedItemsByDistanceFromCenter<BoundedItem>(&node->bound));

		// RI3.A: Remove the last p items from N
		removed_items.assign(node->items.end() - p, node->items.end());
		node->items.erase(node->items.end() - p, node->items.end());

		// RI3.B: adjust the bounding rectangle of N
		node->bound.reset();
		std::for_each(node->items.begin(), node->items.end(), StretchBoundingBox<BoundedItem>(&node->bound));

		//		std::cout << "========2222=======:" << std::endl;
		//		std::cout << "removed_items.size():" << removed_items.size() << std::endl;

		// RI4: In the sort, defined in RI2, starting with the
		// minimum distance (= close reinsert), invoke Insert
		// to reinsert the items
		for (typename std::vector<BoundedItem*>::iterator it = removed_items.begin(); it != removed_items.end(); it++)
			InsertInternal(static_cast<Leaf*>(*it), m_root, false);
	}

	/****************************************************************
	 * These are used to implement walking the entire R* tree in a
	 * conditional way
	 ****************************************************************/

	// visits a node if necessary
	template<typename Acceptor, typename Visitor>
	struct VisitFunctor: std::unary_function<const BoundingBox *, void> {

		const Acceptor &accept;
		Visitor &visit;

		explicit VisitFunctor(const Acceptor &a, Visitor &v) :
				accept(a), visit(v) {
		}

		void operator()(BoundedItem * item) {
			Leaf * leaf = static_cast<Leaf*>(item);

			if (accept(leaf))
				visit(leaf);
		}
	};

	// this functor recursively walks the tree
	template<typename Acceptor, typename Visitor>
	struct QueryFunctor: std::unary_function<const BoundedItem, void> {
		const Acceptor &accept;
		Visitor &visitor;

		explicit QueryFunctor(const Acceptor &a, Visitor &v) :
				accept(a), visitor(v) {
		}

		void operator()(BoundedItem * item) {
			Node * node = static_cast<Node*>(item);

#ifdef QUERY_PROFILING
			Profiling_Counting_RDU::counting ++;
#endif

//			sim_mob::Debug_Static_Count::query_depth ++;
//			std::cout << "one more" << std::endl;

			if (visitor.ContinueVisiting && accept(node)) {
				if (node->hasLeaves)
					for_each(node->items.begin(), node->items.end(), VisitFunctor<Acceptor, Visitor>(accept, visitor));
				else
					for_each(node->items.begin(), node->items.end(), *this);
			}
		}
	};

	// this functor recursively walks the tree
	template<typename Acceptor, typename Visitor>
	struct DebugStructureFunctor: std::unary_function<const BoundedItem, void> {
		const Acceptor &accept;
		Visitor &visitor;

		explicit DebugStructureFunctor(const Acceptor &a, Visitor &v) :
				accept(a), visitor(v) {
		}

		void operator()(BoundedItem * item) {
			Node * node = static_cast<Node*>(item);
			std::cout << node->hasLeaves << ",(" << node->bound.edges[0].first << "," << node->bound.edges[0].second << "," << node->bound.edges[1].first << "," << node->bound.edges[1].second << ")," << node->items.size() << std::endl;

			if (visitor.ContinueVisiting && accept(node)) {
				if (node->hasLeaves)
					for_each(node->items.begin(), node->items.end(), VisitFunctor<Acceptor, Visitor>(accept, visitor));
				else
					for_each(node->items.begin(), node->items.end(), *this);
			}
		}
	};

	/****************************************************************
	 * Used to remove items from the tree
	 *
	 * At some point, the complexity just gets ridiculous. I'm pretty
	 * sure that the remove functions are close to that by now...
	 ****************************************************************/

	// determines whether a leaf should be deleted or not
	template<typename Acceptor, typename LeafRemover>
	struct RemoveLeafFunctor: std::unary_function<const BoundingBox *, bool> {
		const Acceptor &accept;
		LeafRemover &remove;
		std::size_t * size;

		explicit RemoveLeafFunctor(const Acceptor &a, LeafRemover &r, std::size_t * s) :
				accept(a), remove(r), size(s) {
		}

		bool operator()(BoundedItem * item) const {
			Leaf * leaf = static_cast<Leaf *>(item);

			if (accept(leaf) && remove(leaf)) {
				--(*size);
//				delete leaf;
				return true;
			}

			return false;
		}
	};

	template<typename Acceptor, typename LeafRemover>
	struct RemoveFunctor: std::unary_function<const BoundedItem *, bool> {
		const Acceptor &accept;
		LeafRemover &remove;

		// parameters that are passed in
		std::list<Leaf*> * itemsToReinsert;
		std::size_t * m_size;

		// the third parameter is a list that the items that need to be reinserted
		// are put into
		explicit RemoveFunctor(const Acceptor &na, LeafRemover &lr, std::list<Leaf*>* ir, std::size_t * size) :
				accept(na), remove(lr), itemsToReinsert(ir), m_size(size) {
		}

		bool operator()(BoundedItem * item, bool isRoot = false) {
			Node * node = static_cast<Node*>(item);

			if (accept(node)) {
				// this is the easy part: remove nodes if they need to be removed
				if (node->hasLeaves)
					node->items.erase(std::remove_if(node->items.begin(), node->items.end(), RemoveLeafFunctor<Acceptor, LeafRemover>(accept, remove, m_size)), node->items.end());
				else
					node->items.erase(std::remove_if(node->items.begin(), node->items.end(), *this), node->items.end());

				if (!isRoot) {
					if (node->items.empty()) {
						// tell parent to remove us if theres nothing left
						delete node;
						return true;
					} else if (node->items.size() < min_child_items) {
						// queue up the items that need to be reinserted
#ifdef MEAUSURE_COUNTS
						static long merge_counts = 0;
						merge_counts++;
						std::cout << "merge_counts:" << merge_counts << std::endl;
#endif
						QueueItemsToReinsert(node);
						return true;
					}
				} else if (node->items.empty()) {
					// if the root node is empty, setting these won't hurt
					// anything, since the algorithms don't actually require
					// the nodes to have anything in them.
					node->hasLeaves = true;
					node->bound.reset();
				}
			}

			// anything else, don't remove it
			return false;

		}

		// theres probably a better way to do this, but this
		// traverses and finds any leaves, and adds them to a
		// list of items that will later be reinserted
		void QueueItemsToReinsert(Node * node) {
			typename std::vector<BoundedItem*>::iterator it = node->items.begin();
			typename std::vector<BoundedItem*>::iterator end = node->items.end();

			if (node->hasLeaves) {
				for (; it != end; it++)
					itemsToReinsert->push_back(static_cast<Leaf*>(*it));
			} else
				for (; it != end; it++)
					QueueItemsToReinsert(static_cast<Node*>(*it));

			delete node;
		}
	};

public:
	Node * m_root;

	std::size_t m_size;

	std::map<int, Leaf*> second_index;

};

//#endif
