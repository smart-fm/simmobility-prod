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
AMODObjContainer::AMODObjContainer(AMODObj& obj) : value(20), data("Obj1"), obj(obj) {}
AMODEventArgs::AMODEventArgs(AMODObjContainer& obj) : obj(obj) {
	}
}

} /* namespace sim_mob */
