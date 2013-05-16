/*
 * this file is inclusion place for custome Messgae classes
 * whose type will be used in defining templated handlers
 * This generic class is basically assumed to serve as a wrapper around a
 * data string with Json format.
 */

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <iostream>
#include<boost/shared_ptr.hpp>

namespace sim_mob
{
//Forward Declaration
class Handler;
namespace comm
{

typedef int MessageType;

//Base Message
class Message
{
public:
//	Message();
	//todo put hdlr_ptr
	virtual boost::shared_ptr<Handler> supplyHandler() = 0;
};
}//namespace comm
}//namespace sim_mob

typedef boost::shared_ptr<sim_mob::comm::Message> msg_ptr;

#endif
