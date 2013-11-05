//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

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

class JMessage {
public:

	enum MESSAGEID{MSG_INITIALIZE=1, MSG_SIMULATION_SETTINGS=2, MSG_LINKTRAVELUPADTE=3, MSG_REQUEST=4, MSG_OFFER=5, MSG_ACCEPT=6, MSG_CONFIRMATION=7,
		MSG_VEHICLESTOP=81, MSG_VEHICLEPOS=82, MSG_SCHEDULE_FETCH=91, MSG_SCHEDULE=92, MSG_ACK=100};

	JMessage();
	virtual ~JMessage();
	virtual std::string BuildToString();
	virtual void CreateMessage(std::string msg);
	int GetMessageID() { return messageID_; }
	static int GetMessageID(std::string msg);

public:
	std::string msg_;
	int messageID_;
};

class MsgInitialize : public JMessage {
public:
	std::string startTime;
	std::string mapType;
	std::string mapFile;
	std::string mapDirectory;
	int version;

public:
	virtual std::string BuildToString();
};

struct Request
{
	int clientId;
	int origin;
	int destination;
	std::string departureTimeEarly;
	std::string departureTimeLate;
	std::string arrivalTimeEarly;
	std::string arrivalTimeLate;
};

class MsgRequest : public JMessage {
public:
	std::string currentTime;
	Request request;
	int		seatNum;
public:
	virtual std::string BuildToString();
};

class MsgVehicleInit : public JMessage {
public:
	struct SUPPLY
	{
		int vehicleId;
		int nodeId;
	};
	std::vector<SUPPLY> vehicles;
public:
	virtual void CreateMessage(std::string msg);
};

class MsgOffer : public JMessage {
public:
	std::string clientId;
	struct OFFER
	{
		std::string schduleId;
		int serviceType;
		int fare;
		std::string departureTimeEarly;
		std::string depatureTimeLate;
		std::string arivalTimeEarly;
		std::string arrivalTimeLate;
		int travelTime;
		int travelDistance;
	};
	std::vector<OFFER> offers;
public:
	virtual void CreateMessage(std::string msg);
};

class MsgAccept : public JMessage {
public:
	std::string currentTime;
	std::string clientId;
	std::string scheduleId;
	std::string departureTime;
	std::string arrivalTime;
public:
	virtual std::string BuildToString();
};

class MsgConfirmation : public JMessage {
public:
	std::string clientId;
	std::string scheduleId;
public:
	virtual void CreateMessage(std::string msg);
};

class MsgLinkTravel : public JMessage {
public:
	std::string currentTime;
	struct LINK
	{
		int node1Id;
		int node2Id;
		int wayId;
		double travelTime;
	};
	std::vector<LINK> links;
public:
	virtual std::string BuildToString();
};

class MsgVehicleStop : public JMessage {
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

class MsgVehiclePos : public JMessage {
public:
	std::string current_time;
	std::string vehicle_id;
	std::string latitude;
	std::string longtitude;
public:
	virtual std::string BuildToString();
};

class MsgSchedule : public JMessage {
public:
	int vehicle_id;
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


#endif /* MESSAGE_HPP_ */
