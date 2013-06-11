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
#include<json/json.h>
#include "logging/Log.hpp"

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
	hdlr_ptr supplyHandler(){
		return handler;;
	}
	void setHandler( hdlr_ptr handler_)
	{
		handler = handler_;
	}
	T& getData()
	{
		return data;
	}
};
}//namespace comm
//todo do something here. the following std::string is spoiling messge's templatization benefits
typedef Json::Value msg_data_t;
typedef sim_mob::comm::Message<msg_data_t> msg_t;
typedef boost::shared_ptr<msg_t> msg_ptr; //putting std::string here is c++ limitation(old standard). don't blame me!-vahid

}//namespace sim_mob

#endif
