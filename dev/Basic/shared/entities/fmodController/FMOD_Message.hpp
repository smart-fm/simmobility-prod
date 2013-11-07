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

/**
  * package a FMOD message
  */
class FMOD_Message {
public:

	enum MESSAGEID{MSG_INITIALIZE=1, MSG_SIMULATION_SETTINGS=2, MSG_LINKTRAVELUPADTE=3, MSG_REQUEST=4, MSG_OFFER=5, MSG_ACCEPT=6, MSG_CONFIRMATION=7,
		MSG_VEHICLESTOP=81, MSG_VEHICLEPOS=82, MSG_SCHEDULE_FETCH=91, MSG_SCHEDULE=92, MSG_ACK=100};

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
	virtual void createMessage(std::string& msg);

    /**
      * get message id from current FMOD message
      * @return message id.
      */
	int getMessageID() { return messageID_; }

    /**
      * analyze a message id from a json string
      * @return message id.
      */
	static int AnalyzeMessageID(std::string& msg);

public:
	std::string msg_;
	int messageID_;
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
	struct SUPPLY
	{
		int vehicleId;
		int nodeId;
	};
	std::vector<SUPPLY> vehicles;
public:

    /**
      * create message value from a json string
      * @return void.
      */
	virtual void createMessage(std::string& msg);
};

/**
  * package a offer message from FMOD simulator
  */
class MsgOffer : public FMOD_Message {
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

    /**
      * create message value from a json string
      * @return void.
      */
	virtual void createMessage(std::string& msg);
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
	virtual void createMessage(std::string& msg);
};

/**
  * package a link travel time message to FMOD simulator
  */
class MsgLinkTravel : public FMOD_Message {
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
	struct STOP
	{
		std::string stopId;
		std::string arrivalTime;
		std::string depatureTime;
		std::vector< std::string > boardingPassengers;
		std::vector< std::string > alightingPassengers;
	};
	std::vector<STOP> stopSchdules;
	struct PASSENGER
	{
		std::string clientId;
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

    /**
      * create message value from a json string
      * @return void.
      */
	virtual void createMessage(std::string& msg);
};
}

} /* namespace sim_mob */


#endif /* MESSAGE_HPP_ */
