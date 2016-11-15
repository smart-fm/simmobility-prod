/*
 * GeneralR_TreeManager.hpp
 *
 *  Created on: Nov 14, 2016
 *      Author: zhang huai peng
 */

#ifndef GENERALRTREEMANAGER_HPP_
#define GENERALRTREEMANAGER_HPP_
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <vector>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace sim_mob {

using R_Point = bg::model::point<float, 2, bg::cs::cartesian>;
using R_Box = bg::model::box<R_Point>;
template<typename T> using R_Value = std::pair<R_Box, const T*>;
template<typename T> using R_Tree = bgi::rtree<R_Value<T>, bgi::linear<16> >;

template<typename T>
class GeneralR_TreeManager {
public:
	GeneralR_TreeManager();
	virtual ~GeneralR_TreeManager();

	/**
	 * Update all objects into r-tree.
	 * @param objectsForR_Tree is a container including all objects into r-tree
	 */
	void update(const std::set<T*> &objectsForR_Tree);

	/**
	 * Return a collection of objects that are located in the axially-aligned rectangle.
	 * @param lowerLeft The lower left corner of the axially-aligned search rectangle.
	 * @param upperRight The upper right corner of the axially-aligned search rectangle.
	 * @return a collection of objects
	 * The caller is responsible to determine the "type" of each object in the returned array.
	 */
	std::vector<T const *> objectsInBox(const R_Point &lowerLeft, const R_Point &upperRight) const;

private:
	/**internal r-tree to store objects*/
	R_Tree<T>* rTree;
};

}
#endif /* GENERALRTREEMANAGER_HPP_ */
