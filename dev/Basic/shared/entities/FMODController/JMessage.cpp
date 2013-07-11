/*
 * Message.cpp
 *
 *  Created on: May 22, 2013
 *      Author: zhang
 */

#include "JMessage.hpp"
#include "sstream"

namespace sim_mob {

namespace FMOD
{

JMessage::JMessage() : messageID_(-1) {
	// TODO Auto-generated constructor stub

}

JMessage::~JMessage() {
	// TODO Auto-generated destructor stub
}

std::string JMessage::BuildToString()
{
	std::string msg;

	std::stringstream buffer;
	buffer << "message " << this->messageID_ << std::endl;
	msg = buffer.str();

	return msg;
}

int JMessage::GetMessageID(std::string msg)
{
	int ID = -1;
	int index1 = msg.find("message ");
	if( index1 > 0 )
	{
		int index2 = msg.find(",", index1);
		std::string message_id = msg.substr(index1, index2-index2);
		ID = atoi(message_id.c_str());
	}
	return ID;
}

void JMessage::CreateMessage(std::string msg)
{
	messageID_ = GetMessageID( msg );
	msg_ = msg;
}

void Msg_Vehicle_Init::CreateMessage(std::string msg)
{
	JMessage::CreateMessage(msg);

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
		SUPPLY suplier;
		suplier.vehicle_id = item["vehicle_id"].asInt();
		suplier.node_id = item["node_id"].asInt();
		vehicles.push_back(suplier);
	}
}

void Msg_Offer::CreateMessage(std::string msg)
{
	JMessage::CreateMessage(msg);

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

	client_id = root["client_id"].asString();
	Json::Value arrVeh;
	arrVeh = root["offer"];
	for(int i=0; i<arrVeh.size(); i++)
	{
		Json::Value item = arrVeh[i];
		OFFER offer;
		offer.schdule_id = item["schdule_id"].asString();
		offer.service_type = item["service_type"].asInt();
		offer.fare = item["fare"].asInt();
		offer.departure_time_early = item["departure_time_early"].asString();
		offer.depature_time_late = item["depature_time_late"].asString();
		offer.arival_time_early = item["arival_time_early"].asString();
		offer.arrival_time_late = item["arrival_time_late"].asString();
		offer.travel_time = item["travel_time"].asInt();
		offer.travel_distance = item["travel_distance"].asInt();
		offers.push_back(offer);
	}
}

void Msg_Confirmation::CreateMessage(std::string msg)
{
	JMessage::CreateMessage(msg);

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

	client_id = root["client_id"].asString();
	schedule_id = root["schedule_id"].asString();
}

std::string Msg_Initialize::BuildToString()
{
	std::string msg;
	Json::Value Request;
	Request["map"]["type"] = this->map_type;
	Request["map"]["file"] = this->map_file;
	Request["map"]["directory"] = this->map_directory;
	Request["start_time"] = this->start_time;
	Request["version"] = this->version;

	std::stringstream buffer;
	buffer << "message " << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string Msg_Request::BuildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->current_time;
	Request["client_id"] = this->request.client_id;
	Request["orgin"] = this->request.origin;
	Request["destination"] = this->request.destination;
	Request["departure_time_early"] = this->request.departure_time_early;
	Request["depature_time_late"] = this->request.departure_time_late;
	Request["arriavl_time_early"] = this->request.arrival_time_early;
	Request["arrival_time_late"] = this->request.arrival_time_late;
	Request["seat_num"] = this->seat_num;

	std::stringstream buffer;
	buffer << "message " << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string Msg_Accept::BuildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->current_time;
	Request["client_id"] = this->client_id;
	Request["accept"]["schedule_id"] = this->schedule_id;
	Request["accept"]["departure_time"] = this->departure_time;
	Request["accept"]["arrive_time"] = this->arrival_time;

	std::stringstream buffer;
	buffer << "message " << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string Msg_Link_Travel::BuildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->current_time;
	for(int i=0; i<links.size(); i++)
	{
		Json::Value item;
		item["node1_id"] = links[i].node1_id;
		item["node2_id"] = links[i].node2_id;
		item["way_id"] = links[i].way_id;
		item["travel_time"] = links[i].travel_time;
		Request["link"].append(item);
	}

	std::stringstream buffer;
	buffer << "message " << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string Msg_Vehicle_Stop::BuildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->current_time;
	Request["event_type"] = this->event_type;
	Request["shedule_id"] = this->schedule_id;
	Request["stop"] = this->stop_id;
	for(int i=0; i<boarding_passengers.size(); i++)
	{
		Json::Value item = Json::Value(boarding_passengers[i]);
		Request["boarding_passengers"].append(item);
	}

	for(int i=0; i<aligting_passengers.size(); i++)
	{
		Json::Value item = Json::Value(aligting_passengers[i]);
		Request["aligting_passengers"].append(item);
	}

	std::stringstream buffer;
	buffer << "message " << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string Msg_Vehicle_Pos::BuildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->current_time;
	Request["vehicle_id"] = this->vehicle_id;
	Request["latitude"] = this->latitude;
	Request["longtitude"] = this->longtitude;

	std::stringstream buffer;
	buffer << "message " << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

void Msg_Schedule::CreateMessage(std::string msg)
{
	JMessage::CreateMessage(msg);

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

	vehicle_id = root["vehicle_id"].asInt();
	schedule_id = root["schedule_id"].asString();
	service_type = root["service_type"].asInt();

	Json::Value arrStops = root["stop_schdules"];
	for(int i=0; i<arrStops.size(); i++)
	{
		Json::Value item = arrStops[i];
		STOP stop;
		stop.stop_id = item["stop"].asString();
		stop.arrival_time = item["arrival_time"].asString();
		stop.depature_time = item["depature_time"].asString();
		Json::Value arrPassengers = item["board_passengers"];
		for(int k=0; k<arrPassengers.size(); k++){
			Json::Value val = arrPassengers[k];
			stop.boardingpassengers.push_back(val.asString());
		}
		arrPassengers = item["alight_passengers"];
		for(int k=0; k<arrPassengers.size(); k++){
			Json::Value val = arrPassengers[k];
			stop.alightingpassengers.push_back(val.asString());
		}
		stop_schdules.push_back(stop);
	}

	Json::Value arrPassengers = root["passengers"];
	for(int i=0; i<arrStops.size(); i++)
	{
		Json::Value item = arrPassengers[i];
		PASSENGER pass;
		pass.client_id = item["client_id"].asString();
		pass.price = item["price"].asInt();
		passengers.push_back(pass);
	}

	Json::Value arrRoutes = root["route"];
	for(int i=0; i<arrStops.size(); i++)
	{
		Json::Value item = arrRoutes[i];
		ROUTE route;
		route.id = item["id"].asString();
		route.type = item["type"].asInt();
		routes.push_back(route);
	}
}
}

} /* namespace sim_mob */
