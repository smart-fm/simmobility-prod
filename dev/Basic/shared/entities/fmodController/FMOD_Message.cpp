//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Message.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "FMOD_Message.hpp"
#include "sstream"
#include <boost/lexical_cast.hpp>

namespace sim_mob {

namespace FMOD
{

FMOD_Message::FMOD_Message() : messageID_(MSG_DEFALUTVALUE) {
	// TODO Auto-generated constructor stub

}

FMOD_Message::~FMOD_Message() {
	// TODO Auto-generated destructor stub
}

std::string FMOD_Message::buildToString()
{
	std::string msg;

	std::stringstream buffer;
	buffer << this->messageID_ << std::endl;
	msg = buffer.str();

	return msg;
}

FMOD_Message::FMOD_MessageID FMOD_Message::analyzeMessageID(const std::string& msg)
{
	FMOD_MessageID ID = MSG_DEFALUTVALUE;
	int index1 = 0; //msg.find("message ");
	if( index1 >= 0 )
	{
		int index2 = msg.find(",", index1);
		std::string message_id = msg.substr(index1, index2-index1);
		ID = (FMOD_MessageID)atoi(message_id.c_str());
	}
	return ID;
}

void FMOD_Message::createMessage(const std::string& msg)
{
	messageID_ = analyzeMessageID( msg );
	msg_ = msg;
}

void MsgVehicleInit::createMessage(const std::string& msg)
{
	FMOD_Message::createMessage(msg);

	int index = msg.find("{");
	if(index < 0)
		return;

	std::string jsonstr = msg.substr(index, msg.size()-index);
	Json::Value root;
	Json::Reader reader;

	bool parsing = reader.parse(jsonstr, root);
	if( !parsing )
	{
		std::cout << "Failded to parse string" << std::endl;
		return;
	}

	Json::Value arrVeh;
	arrVeh = root["supply"];
	vehicles.clear();
	for(int i=0; i<arrVeh.size(); i++)
	{
		Json::Value item = arrVeh[i];
		Supply suplier;
		suplier.vehicleId = boost::lexical_cast<int>(item["vehicle_id"].asCString());
		suplier.nodeId = boost::lexical_cast<int>(item["node_id"].asCString());
		vehicles.push_back(suplier);
	}
}

void MsgOffer::createMessage(const std::string& msg)
{
	FMOD_Message::createMessage(msg);

	int index = msg.find("{");
	if(index < 0)
		return;

	std::string jsonstr = msg.substr(index, msg.size()-index);
	Json::Value root;
	Json::Reader reader;

	bool parsing = reader.parse(jsonstr, root);
	if( !parsing )
	{
		std::cout << "Failded to parse string" << std::endl;
		return;
	}

	clientId = root["client_id"].asString();
	Json::Value arrVeh;
	arrVeh = root["offer"];
	for(int i=0; i<arrVeh.size(); i++)
	{
		Json::Value item = arrVeh[i];
		Offer offer;
		offer.schduleId = item["schdule_id"].asString();
		offer.serviceType = boost::lexical_cast<int>(item["service_type"].asCString());
		offer.fare = item["fare"].asInt();
		offer.departureTimeEarly = item["departure_time_early"].asString();
		offer.depatureTimeLate = item["depature_time_late"].asString();
		offer.arivalTimeEarly = item["arival_time_early"].asString();
		offer.arrivalTimeLate = item["arrival_time_late"].asString();
		offer.travelTime = boost::lexical_cast<int>(item["travel_time"].asCString());
		offer.travelDistance = boost::lexical_cast<int>(item["travel_distance"].asCString());
		offers.push_back(offer);
	}
}

void MsgConfirmation::createMessage(const std::string& msg)
{
	FMOD_Message::createMessage(msg);

	int index = msg.find("{");
	if(index < 0)
		return;

	std::string jsonstr = msg.substr(index, msg.size()-index);
	Json::Value root;
	Json::Reader reader;

	bool parsing = reader.parse(jsonstr, root);
	if( !parsing )
	{
		std::cout << "Failded to parse string" << std::endl;
		return;
	}

	clientId = root["client_id"].asString();
	scheduleId = root["schedule_id"].asString();
}

std::string MsgInitialize::buildToString()
{
	std::string msg;
	Json::Value Request;
	Request["map"]["type"] = this->mapType;
	Request["map"]["file"] = this->mapFile;
	Request["map"]["directory"] = this->mapDirectory;
	Request["start_time"] = this->startTime;
	Request["version"] = this->version;

	std::stringstream buffer;
	buffer << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string MsgRequest::buildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->currentTime;
	Request["client_id"] = boost::lexical_cast<std::string>(this->request.clientId);
	Request["origin"] = boost::lexical_cast<std::string>(this->request.origin);
	Request["destination"] = boost::lexical_cast<std::string>(this->request.destination);
	Request["departure_time_early"] = this->request.departureTimeEarly;
	Request["depature_time_late"] = this->request.departureTimeLate;
	Request["arriavl_time_early"] = this->request.arrivalTimeEarly;
	Request["arrival_time_late"] = this->request.arrivalTimeLate;
	Request["seat_num"] = boost::lexical_cast<std::string>(this->seatNum);

	std::stringstream buffer;
	buffer << this->messageID_ << ",0," << Request << std::endl;
	msg = buffer.str();

	//std::cout<< msg << std::endl;

	return msg;
}

std::string MsgAccept::buildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->currentTime;
	Request["client_id"] = this->clientId;
	Request["accept"]["schedule_id"] = this->scheduleId;
	Request["accept"]["departure_time"] = this->departureTime;
	Request["accept"]["arrive_time"] = this->arrivalTime;

	std::stringstream buffer;
	buffer << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string MsgLinkTravel::buildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->currentTime;
	for(int i=0; i<links.size(); i++)
	{
		Json::Value item;
		item["node1_id"] = links[i].node1Id;
		item["node2_id"] = links[i].node2Id;
		item["way_id"] = links[i].wayId;
		item["travel_time"] = links[i].travelTime;
		Request["link"].append(item);
	}

	std::stringstream buffer;
	buffer << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string MsgVehicleStop::buildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->currentTime;
	Request["event_type"] = this->eventType;
	Request["shedule_id"] = this->scheduleId;
	Request["stop"] = this->stopId;
	for(int i=0; i<boardingPassengers.size(); i++)
	{
		Json::Value item = Json::Value(boardingPassengers[i]);
		Request["boarding_passengers"].append(item);
	}

	for(int i=0; i<aligtingPassengers.size(); i++)
	{
		Json::Value item = Json::Value(aligtingPassengers[i]);
		Request["aligting_passengers"].append(item);
	}

	std::stringstream buffer;
	buffer << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string MsgVehiclePos::buildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->currentTime;
	Request["vehicle_id"] = this->vehicleId;
	Request["latitude"] = this->latitude;
	Request["longtitude"] = this->longtitude;

	std::stringstream buffer;
	buffer << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

void MsgSchedule::createMessage(const std::string& msg)
{
	FMOD_Message::createMessage(msg);

	int index = msg.find("{");
	if(index < 0)
		return;

	std::string jsonstr = msg.substr(index, msg.size()-index);
	Json::Value root;
	Json::Reader reader;

	bool parsing = reader.parse(jsonstr, root);
	if( !parsing )
	{
		std::cout << "Failded to parse string" << std::endl;
		return;
	}

	vehicleId = boost::lexical_cast<int>(root["vehicle_id"].asCString());
	scheduleId = root["schedule_id"].asString();
	serviceType = boost::lexical_cast<int>(root["service_type"].asCString());

	Json::Value arrStops = root["stop_schdules"];
	for(int i=0; i<arrStops.size(); i++)
	{
		Json::Value item = arrStops[i];
		Stop stop;
		stop.stopId = item["stop"].asString();
		stop.arrivalTime = item["arrival_time"].asString();
		stop.depatureTime = item["depature_time"].asString();
		Json::Value arrPassengers = item["board_passengers"];
		for(int k=0; k<arrPassengers.size(); k++){
			Json::Value val = arrPassengers[k];
			stop.boardingPassengers.push_back(val.asString());
		}
		arrPassengers = item["alight_passengers"];
		for(int k=0; k<arrPassengers.size(); k++){
			Json::Value val = arrPassengers[k];
			stop.alightingPassengers.push_back(val.asString());
		}
		stopSchdules.push_back(stop);
	}

	Json::Value arrPassengers = root["passengers"];
	for(int i=0; i<arrStops.size(); i++)
	{
		Json::Value item = arrPassengers[i];
		Passenger pass;
		pass.clientId = item["client_id"].asString();
		pass.price = boost::lexical_cast<int>(item["price"].asCString());
		passengers.push_back(pass);
	}

	Json::Value arrRoutes = root["route"];
	for(int i=0; i<arrStops.size(); i++)
	{
		Json::Value item = arrRoutes[i];
		Route route;
		route.id = item["id"].asString();
		route.type = boost::lexical_cast<int>(item["type"].asCString());
		routes.push_back(route);
	}
}


std::string MsgFinalize::buildToString(){
	std::string msg;
	Json::Value Request;
	Request["end_time"] = this->end_time;

	std::stringstream buffer;
	buffer << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}


}

} /* namespace sim_mob */
