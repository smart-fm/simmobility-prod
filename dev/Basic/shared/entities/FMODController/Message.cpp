/*
 * Message.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "Message.hpp"
#include "sstream"

namespace sim_mob {

namespace FMOD
{

Message::Message() {
	// TODO Auto-generated constructor stub

}

Message::~Message() {
	// TODO Auto-generated destructor stub
}

std::string Msg_Request::BuildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->current_time;
	Request["client_id"] = this->client_id;
	Request["orgin"] = this->origin;
	Request["destination"] = this->destination;
	Request["departure_time_early"] = this->departure_time_early;
	Request["depature_time_late"] = this->departure_time_late;
	Request["seat_num"] = this->seat_num;

	std::stringstream buffer;
	buffer << "message " << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

}

} /* namespace sim_mob */
