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

class AMODObjContainer {
public:
	AMODObjContainer(AMODObj& obj);
	AMODObj& obj;
	int value;
	std::string data;
};

class AMODEventArgs : public event::EventArgs {
public:

	AMODEventArgs(AMODObjContainer& obj);
	AMODObjContainer& obj;
};


}
} /* namespace sim_mob */
#endif /* AMODEVENT_HPP_ */
