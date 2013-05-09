/*
 * HandlerFactory.hpp
 *
 *  Created on: May 7, 2013
 *      Author: vahid
 */

#ifndef HANDLERFACTORY_HPP_
#define HANDLERFACTORY_HPP_



namespace sim_mob {
namespace comm
{
class Message;
}
class Handler;
//my Base Handler Factory

class HandlerFactory {
public:
	Handler * create(sim_mob::comm::Message * message);
};

} /* namespace sim_mob */
#endif /* HANDLERFACTORY_HPP_ */
