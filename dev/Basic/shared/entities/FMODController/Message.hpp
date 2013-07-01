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
#include <boost/shared_ptr.hpp>
#include <json/json.h>

namespace sim_mob {

namespace FMOD
{

class Message {
public:

	enum MESSAGEID{MSG_INITIALIZE=1, MSG_SIMULATION_SETTINGS=2, MSG_LINKTRAVELUPADTE=3, MSG_REQUEST=4, MSG_OFFER=5, MSG_ACCEPT=6, MSG_CONFIRM=7,
		MSG_VEHICLESTOP=81, MSG_VEHICLEPOS=82, MSG_SCHEDULE_FETCH=91, MSG_SCHEDULE=92};

	Message();
	virtual ~Message();
	virtual std::string BuildToString() { return msg_;}
	virtual void CreateMessage(std::string msg);
	int GetMessageID() { return messageID_; }
	static int GetMessageID(std::string msg);

public:
	std::string msg_;
	int messageID_;
};

class Msg_Initialize : public Message {
public:
	std::string start_time;
	std::string map_type;
	std::string map_file;
	std::string map_directory;
	int version;

public:
	virtual std::string BuildToString();
};

struct Request
{
	int client_id;
	int origin;
	int destination;
	std::string departure_time_early;
	std::string departure_time_late;
	std::string arrival_time_early;
	std::string arrival_time_late;
};

class Msg_Request : public Message {
public:
	std::string current_time;
	Request request;
	int		seat_num;
public:
	virtual std::string BuildToString();
};

class Msg_Vehicle_Init : public Message {
public:
	struct SUPPLY
	{
		std::string vehicle_id;
		std::string node_id;
	};
	std::vector<SUPPLY> vehicles;
public:
	virtual void CreateMessage(std::string msg);
};

class Msg_Offer : public Message {
public:
	std::string client_id;
	struct OFFER
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
	std::vector<OFFER> offers;
public:
	virtual void CreateMessage(std::string msg);
};

class Msg_Accept : public Message {
public:
	std::string current_time;
	std::string client_id;
	std::string schedule_id;
	std::string departure_time;
	std::string arrival_time;
public:
	virtual std::string BuildToString();
};

class Msg_Confirmation : public Message {
public:
	std::string client_id;
	std::string schedule_id;
public:
	virtual void CreateMessage(std::string msg);
};

class Msg_Link_Travel : public Message {
public:
	std::string current_time;
	struct LINK
	{
		std::string node1_id;
		std::string node2_id;
		std::string way_id;
		int travel_time;
	};
	std::vector<LINK> links;
public:
	virtual std::string BuildToString();
};

class Msg_Vehicle_Stop : public Message {
public:
	std::string current_time;
	std::string vehicle_id;
	int	event_type;
	std::string schedule_id;
	std::string stop_id;
	std::vector<int> boarding_passengers;
	std::vector<int> aligting_passengers;
public:
	virtual std::string BuildToString();
};

class Msg_Vehicle_Pos : public Message {
public:
	std::string current_time;
	std::string vehicle_id;
	std::string latitude;
	std::string longtitude;
public:
	virtual std::string BuildToString();
};

class Msg_Schedule : public Message {
public:
	std::string vehicle_id;
	std::string schedule_id;
	std::vector<std::string> replace_schedules;
	int service_type;
	struct STOP
	{
		std::string stop_id;
		std::string arrival_time;
		std::string depature_time;
		std::vector< std::string > boardingpassengers;
		std::vector< std::string > alightingpassengers;
	};
	std::vector<STOP> stop_schdules;
	struct PASSENGER
	{
		std::string client_id;
		int price;
	};
	std::vector<PASSENGER> passengers;
	struct ROUTE
	{
		std::string id;
		int type;
	};
	std::vector<ROUTE> routes;
public:
	virtual void CreateMessage(std::string msg);

};
}

} /* namespace sim_mob */

typedef boost::shared_ptr<sim_mob::FMOD::Message> msg_ptr;

#endif /* MESSAGE_HPP_ */
