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
typedef boost::shared_ptr<sim_mob::Handler> hdlr_ptr;
namespace comm
{

typedef int MessageType;

//Base Message
template<class T>
class Message
{
	T data;
	hdlr_ptr handler;
public:
	Message();
	Message(T data_):data(data_){}
	virtual hdlr_ptr supplyHandler() = 0;
	void setHandler( hdlr_ptr handler_)
	{
		handler = handler_;
	}
};
}//namespace comm
//todo do simething here. the following std::string is spoiling messge's templatization benefits
typedef boost::shared_ptr<sim_mob::comm::Message<std::string> > msg_ptr; //putting std::string here is c++ limitation(old standard). don't blame me!-vahid

}//namespace sim_mob

#endif
