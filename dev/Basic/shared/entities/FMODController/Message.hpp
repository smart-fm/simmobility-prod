/*
 * Message.hpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_
#include "string"
#include "vector"
#include <iostream>
#include<boost/shared_ptr.hpp>
namespace sim_mob {

namespace FMOD
{

class Message {
public:
	Message();
	virtual ~Message();
	virtual std::string BuildToString();
private:
	std::string msg;
	int messageID;
};

class Msg_Request : public Message {
public:
	std::string current_time;
	std::string client_id;
	std::string origin;
	std::string destination;
	std::string departure_time_early;
	std::string departure_time_late;
	int		seat_num;
public:
	virtual std::string BuildToString();

};

class Msg_Vehicle_Init : public Message {
public:
	struct Supply_object
	{
		std::string vehicle_id;
		std::string node_id;
	};
	std::vector<Supply_object> vehicles;
};

class Msg_Offer : public Message {
public:
	std::string client_id;
	struct Offer
	{
		std::string schdule_id;
		int service_type;
		int fare;
		std::string departure_time_early;
		std::string depature_time_late;
		std::string arival_time_early;
		std::string arrival_time_late;
		int travel_time;
		int travel_distance;
	};
	std::vector<Offer> offers;
};

class Msg_Accept : public Message {
public:
	std::string current_time;
	std::string schedule_id;
	std::string departure_time;
	std::string arrival_time;
};
}

} /* namespace sim_mob */

typedef boost::shared_ptr<sim_mob::FMOD::Message> msg_ptr;

#endif /* MESSAGE_HPP_ */
