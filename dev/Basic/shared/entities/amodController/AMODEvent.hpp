/*
 * AMODEvent.hpp
 *
 *  Created on: Apr 17, 2014
 *      Author: Max
 */

#ifndef AMODEVENT_HPP_
#define AMODEVENT_HPP_
#include "event/args/EventArgs.hpp"
#include "event/EventPublisher.hpp"
#include "geospatial/Node.hpp"
#include "geospatial/streetdir/WayPoint.hpp"
#include <vector>
namespace sim_mob {

namespace AMOD {

class AMODEventPublisher: public sim_mob::event::EventPublisher
{
public:
	AMODEventPublisher():id("amod controller") {}
	~AMODEventPublisher(){}

	std::string id;
};

class AMODObj {
public:

	AMODObj();
	int value;
	std::string data;
};

//class AMODObjContainer {
//public:
//	AMODObjContainer(AMODObj& obj);
//	AMODObj& obj;
//	int value;
//	std::string data;
//};
class AMODEventArgs : public event::EventArgs
{
public:
	AMODEventArgs(){}
};
class AMODRerouteEventArgs : public AMODEventArgs {
public:

	AMODRerouteEventArgs(sim_mob::Node *s,sim_mob::Node *e,std::vector<sim_mob::WayPoint> rPath);
	// if reRoutePath is empty, driver will generate shortest path base on reRouteStartNode and reRouteEndNode
	// new path start node,
	sim_mob::Node *reRouteStartNode;
	// new path end node
	sim_mob::Node *reRouteEndNode;
	// if reRoutePath is not empty, drive reroute with the path
	std::vector<sim_mob::WayPoint> reRoutePath;
};


}
} /* namespace sim_mob */
#endif /* AMODEVENT_HPP_ */
