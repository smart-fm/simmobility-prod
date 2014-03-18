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
#include "event/SystemEvents.hpp"
#include "event/args/EventArgs.hpp"
#include "message/MessageBus.hpp"
#include "event/EventPublisher.hpp"

namespace sim_mob {

/**
 * Subclasses both EventArgs, This is to allow it to function as an Event callback parameter.
 */
class FMODSchedule;
class FMOD_RequestEventArgs : public sim_mob::event::EventArgs {
public:
	FMOD_RequestEventArgs(FMODSchedule* sch):schedule(sch){;}
	virtual ~FMOD_RequestEventArgs() {}
	FMODSchedule* schedule;
};

//subclassed Eventpublisher coz its destructor is pure virtual
class FMOD_Publisher: public sim_mob::event::EventPublisher {
public:
	FMOD_Publisher() {
	}
	virtual ~FMOD_Publisher() {
	}
};

namespace FMOD
{
enum {
	EVENT_DISPATCH_REQUEST = 4000000
};

/**
  * package a FMOD message
  */
class FMOD_Message {
public:

	enum FMOD_MessageID{MSG_DEFALUTVALUE=0, MSG_INITIALIZE=1, MSG_SIMULATION_SETTINGS=2, MSG_LINKTRAVELUPADTE=3, MSG_REQUEST=4, MSG_OFFER=5, MSG_ACCEPT=6, MSG_CONFIRMATION=7,
		MSG_VEHICLESTOP=81, MSG_VEHICLEPOS=82, MSG_SCHEDULE_FETCH=91, MSG_SCHEDULE=92, MSG_ACK=999, MSG_FINALIZE=11};

	FMOD_Message();
	virtual ~FMOD_Message();

    /**
      * build a json string from message value.
      * @return final string which contain FMOD message.
      */
	virtual std::string buildToString();

    /**
      * create message value from a json string
      * @return void.
      */
	virtual void createMessage(const std::string& msg);

    /**
      * get message id from current FMOD message
      * @return message id.
      */
	FMOD_MessageID getMessageID() { return messageID_; }

    /**
      * analyze a message id from a json string
      * @return message id.
      */
	static FMOD_MessageID analyzeMessageID(const std::string& msg);

public:
	std::string msg_;
	FMOD_MessageID messageID_;
};

/**
  * package a initialization message from FMOD simulator
  */
class MsgInitialize : public FMOD_Message {
public:
	std::string startTime;
	std::string mapType;
	std::string mapFile;
	std::string mapDirectory;
	int version;

public:
    /**
      * build a json string from message value.
      * @return final string which contain FMOD message.
      */
	virtual std::string buildToString();
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

/**
  * package a request message transfered to FMOD simulator
  */
class MsgRequest : public FMOD_Message {
public:
	std::string currentTime;
	Request request;
	int		seatNum;
public:
    /**
      * build a json string from message value.
      * @return final string which contain FMOD message.
      */
	virtual std::string buildToString();
};

/**
  * package a vehicles initializing message from FMOD simulator
  */
class MsgVehicleInit : public FMOD_Message {
public:
	struct Supply
	{
		int vehicleId;
		int nodeId;
	};
	std::vector<Supply> vehicles;
public:

    /**
      * create message value from a json string
      * @return void.
      */
	virtual void createMessage(const std::string& msg);
};

/**
  * package a offer message from FMOD simulator
  */
class MsgOffer : public FMOD_Message {
public:
	std::string clientId;
	struct Offer
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
	std::vector<Offer> offers;
public:

    /**
      * create message value from a json string
      * @return void.
      */
	virtual void createMessage(const std::string& msg);
};

/**
  * package a acceptance message from FMOD simulator
  */
class MsgAccept : public FMOD_Message {
public:
	std::string currentTime;
	std::string clientId;
	std::string scheduleId;
	std::string departureTime;
	std::string arrivalTime;

public:

    /**
      * build a json string from message value.
      * @return final string which contain FMOD message.
      */
	virtual std::string buildToString();
};

/**
  * package a confirmation message to FMOD simulator
  */
class MsgConfirmation : public FMOD_Message {
public:
	std::string clientId;
	std::string scheduleId;
public:

    /**
      * create message value from a json string
      * @return void.
      */
	virtual void createMessage(const std::string& msg);
};

/**
  * package a link travel time message to FMOD simulator
  */
class MsgLinkTravel : public FMOD_Message {
public:
	std::string currentTime;
	struct Link
	{
		int node1Id;
		int node2Id;
		int wayId;
		double travelTime;
	};
	std::vector<Link> links;
public:
    /**
      * build a json string from message value.
      * @return final string which contain FMOD message.
      */
	virtual std::string buildToString();
};

/**
  * package a vehicles stopping message to FMOD simulator
  */
class MsgVehicleStop : public FMOD_Message {
public:
	std::string currentTime;
	std::string vehicleId;
	int	eventType;
	std::string scheduleId;
	std::string stopId;
	std::vector<int> boardingPassengers;
	std::vector<int> aligtingPassengers;
public:

    /**
      * build a json string from message value.
      * @return final string which contain FMOD message.
      */
	virtual std::string buildToString();
};

/**
  * package a vehicles position message to FMOD simulator
  */
class MsgVehiclePos : public FMOD_Message {
public:
	std::string currentTime;
	std::string vehicleId;
	std::string latitude;
	std::string longtitude;
public:
    /**
      * build a json string from message value.
      * @return final string which contain FMOD message.
      */
	virtual std::string buildToString();
};

/**
  * package a vehicles schedule message from FMOD simulator
  */
class MsgSchedule : public FMOD_Message {
public:
	int vehicleId;
	std::string scheduleId;
	std::vector<std::string> replaceSchedules;
	int serviceType;
	struct Stop
	{
		std::string stopId;
		std::string arrivalTime;
		std::string depatureTime;
		std::vector< std::string > boardingPassengers;
		std::vector< std::string > alightingPassengers;
	};
	std::vector<Stop> stopSchdules;
	struct Passenger
	{
		std::string clientId;
		int price;
	};
	std::vector<Passenger> passengers;
	struct Route
	{
		std::string id;
		int type;
	};
	std::vector<Route> routes;
public:

    /**
      * create message value from a json string
      * @return void.
      */
	virtual void createMessage(const std::string& msg);
};

class MsgFinalize : public FMOD_Message {
public:
	std::string end_time;

public:
    /**
      * build a json string from message value.
      * @return final string which contain FMOD message.
      */
	virtual std::string buildToString();
};
}

} /* namespace sim_mob */


#endif /* MESSAGE_HPP_ */
