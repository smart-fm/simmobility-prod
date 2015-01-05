/*
 * AMODEvent.cpp
 *
 *  Created on: Apr 17, 2014
 *      Author: Max
 */

#include "AMODEvent.hpp"

namespace sim_mob {

namespace AMOD {

AMODObj::AMODObj() : value(10), data("Obj")
{
}
//AMODObjContainer::AMODObjContainer(AMODObj& obj) : value(20), data("Obj1"), obj(obj) {}
AMODRerouteEventArgs::AMODRerouteEventArgs(sim_mob::Node *s,sim_mob::Node *e,std::vector<sim_mob::WayPoint> rPath)
: reRouteStartNode(s),reRouteEndNode(e),reRoutePath(rPath)
{
}
//AMODRerouteEventArgs& AMODRerouteEventArgs::operator=(const AMODRerouteEventArgs& source) {
//    EventArgs::operator =(source);
//    this->reRouteStartNode = source.reRouteStartNode;
//    this->reRouteEndNode = source.reRouteEndNode;
//    this->reRoutePath = source.reRoutePath;
//    return *this;
//}



}

} /* namespace sim_mob */
