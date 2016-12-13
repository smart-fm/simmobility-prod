/*
 * GeneralR_TreeManager.cpp
 *
 *  Created on: Nov 14, 2016
 *      Author: zhang huai peng
 */
#ifndef _CLASS_RTREE_GENERAL_FUNCTIONS
#include "spatial_trees/GeneralR_TreeManager.hpp"
#else
namespace sim_mob {
template<typename T>
GeneralR_TreeManager<T>::GeneralR_TreeManager() : rTree(nullptr)
{

}

template<typename T>
GeneralR_TreeManager<T>::~GeneralR_TreeManager()
{

}

template<typename T>
void GeneralR_TreeManager<T>::update(const std::set<T*> &objectsForR_Tree)
{
	if (rTree) {
		rTree->clear();
		delete rTree;
		rTree = nullptr;
	}

	std::vector<R_Value<T>> objects;
	for (auto itr = objectsForR_Tree.begin(); itr != objectsForR_Tree.end(); ++itr) {
		R_Point location((*itr)->getPosX(), (*itr)->getPosY());
		R_Box box(location, location);
		objects.push_back(std::make_pair(box, (*itr)));
	}

	rTree = new R_Tree<T>(objects.begin(), objects.end());
}

template<typename T>
std::vector<T const *> GeneralR_TreeManager<T>::objectsInBox(const R_Point &lowerLeft, const R_Point &upperRight) const
{
	R_Box queryBox(lowerLeft, upperRight);
	std::vector<R_Value<T>> result;
	if (rTree) {
		rTree->query(bgi::intersects(queryBox), std::back_inserter(result));
	}

	std::vector<const T*> objectsInBox;
	for (auto item : result) {
		objectsInBox.push_back(item.second);
	}

	return objectsInBox;
}

template<typename T>
const T* GeneralR_TreeManager<T>::searchNearestObject(double x, double y) const
{
	//searching region is inside 5km
	const double distance = 5000;
	R_Point lowerLeft(x-distance,y-distance);
	R_Point upperRight(x+distance,y+distance);
	const T* res = nullptr;
	double dis = distance*distance;
	std::vector<T const *> objects = objectsInBox(lowerLeft, upperRight);
	for(auto it=objects.begin(); it!=objects.end(); it++) {
		double r = ((*it)->getPosX()-x)*((*it)->getPosX()-x)+((*it)->getPosY()-y)*((*it)->getPosY()-y);
		if(r<dis) {
			dis = r;
			res = (*it);
		}
	}
	return res;
}
}
#endif
