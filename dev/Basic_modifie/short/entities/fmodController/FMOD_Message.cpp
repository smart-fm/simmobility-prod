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
#include "util/DailyTime.hpp"
#include "conf/ConfigManager.hpp"
#include "conf/ConfigParams.hpp"
#include "geospatial/streetdir/StreetDirectory.hpp"
#include "geospatial/network/RoadNetwork.hpp"
#include "entities/Person.hpp"

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
		offer.scheduleId = item["schedule_id"].asString();
		offer.serviceType = item["service_type"].asDouble();
		offer.fare = item["fare"].asDouble();
		offer.pickupLongitude = item["pickup_latitude"].asString();
		offer.pickupLatitude = item["pickup_longitude"].asString();
		offer.dropoffLongitude = item["dropoff_longitude"].asString();
		offer.dropoffLatitude = item["dropoff_latitude"].asString();
		offer.departureTimeEarly = item["pickup_time_early"].asString();
		offer.depatureTimeLate = item["pickup_time_late"].asString();
		offer.arivalTimeEarly = item["dropoff_time_early"].asString();
		offer.arrivalTimeLate = item["dropoff_time_late"].asString();
		offer.travelTime = item["travel_time"].asDouble();
		offer.travelDistance = item["travel_distance"].asDouble();
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
	requestId = root["request_id"].asString();
	productId = root["product_id"].asString();
	result = root["result"].asInt();
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
	Request["origin_longitude"] = this->request.originLongitude;
	Request["origin_latitude"] = this->request.originLatitude;
	Request["destination_longitude"] = this->request.destLongitude;
	Request["destination_latitude"] = this->request.destLatitude;
	Request["departure_time_early"] = this->request.departureTimeEarly;
	Request["departure_time_late"] = this->request.departureTimeLate;
	Request["arrival_time_early"] = this->request.arrivalTimeEarly;
	Request["arrival_time_late"] = this->request.arrivalTimeLate;
	Request["seat_num"] = this->request.seatNum;

	std::stringstream buffer;
	buffer << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

std::string MsgAccept::buildToString()
{
	std::string msg;
	Json::Value Request;
	Request["current_time"] = this->currentTime;
	Request["client_id"] = this->clientId;
	Request["request_id"] = this->requestId;
	Request["accept"]["product_id"] = this->accept.productId;
	Request["accept"]["pickup_time"] = this->accept.pickupTime;
	Request["accept"]["dropoff_time"] = this->accept.dropoffTime;

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
		item["node1_id"] = boost::lexical_cast<std::string>(links[i].node1Id);
		item["node2_id"] = boost::lexical_cast<std::string>(links[i].node2Id);
		item["way_id"] = boost::lexical_cast<std::string>(links[i].wayId);
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
	Request["schedule_id"] = this->scheduleId;
	Request["vehicle_id"] = this->vehicleId;
	Request["stop"] = this->stopId;
	Request["board_passengers"].clear();
	for(int i=0; i<boardingPassengers.size(); i++)
	{
		Json::Value item = Json::Value(boost::lexical_cast<std::string>(boardingPassengers[i]));
		Request["board_passengers"].append(item);
	}

	Request["alight_passengers"].clear();
	for(int i=0; i<aligtingPassengers.size(); i++)
	{
		Json::Value item = Json::Value(boost::lexical_cast<std::string>(aligtingPassengers[i]));
		Request["alight_passengers"].append(item);
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
	Request["longitude"] = this->longtitude;

	std::stringstream buffer;
	buffer << this->messageID_ << "," << Request << std::endl;
	msg = buffer.str();

	return msg;
}

int MsgSchedule::getVehicleId()
{
	int ret = -1;
	if(schedules.size()>0){
		ret = schedules.front().vehicleId;
	}

	return ret;
}

unsigned int MsgSchedule::getStartTime()
{
	int ret = 0;

	if(schedules.size()>0 && schedules.front().stopSchdules.size()>0){
		sim_mob::DailyTime start(schedules.front().stopSchdules[0].depatureTime);
		ret = start.getValue();
	}

	return ret;
}

Node* MsgSchedule::getStartNode()
{
	Node* ret = nullptr;
	if(schedules.size()>0 && schedules.front().routes.size()>0){
		ret = (schedules.front().routes.front());
	}
	return ret;
}

Node* MsgSchedule::getEndNode()
{
	Node* ret = nullptr;
	if(schedules.size()>0 && schedules.back().routes.size()>0){
		ret = (schedules.back().routes.back());
	}
	return ret;
}

std::string MsgSchedule::buildToString()
{
	std::string msg;
	Json::Value request, result;

	std::list<FMOD_Schedule>::iterator it;
	for(it=schedules.begin(); it!=schedules.end(); it++){
		request["vehicle_id"] = (*it).vehicleId;
		request["schedule_id"] = (*it).scheduleId;
		request["serviceType"] = (*it).serviceType;
		request["status"] = (*it).status;
		request["routes"].clear();
		for(std::vector<Node*>::iterator itRoute=(*it).routes.begin(); itRoute != (*it).routes.end(); itRoute++){
			Json::Value item = Json::Value((*itRoute)->getNodeId());
			request["routes"].append(item);
		}

		request["stop_schedules"].clear();
		Json::Value item;
		for(std::vector<FMOD_Schedule::Stop>::iterator itStop=(*it).stopSchdules.begin(); itStop!=(*it).stopSchdules.end(); itStop++){
			item["latitude"] = itStop->latitude;
			item["longitude"] = itStop->longitude;
			item["depature_time"] = itStop->depatureTime;
			item["arrive_time"] = itStop->arrivalTime;
			item["boarding_passenger"].clear();
			item["aligting_passenger"].clear();
			for(std::vector<int>::iterator itPassager=itStop->boardingPassengers.begin();itPassager!=itStop->boardingPassengers.end();itPassager++){
				Json::Value passenger = Json::Value(*itPassager);
				item["boarding_passenger"].append(passenger);
			}
			for(std::vector<int>::iterator itPassager=itStop->alightingPassengers.begin();itPassager!=itStop->alightingPassengers.end();itPassager++){
				Json::Value passenger = Json::Value(*itPassager);
				item["aligting_passenger"].append(passenger);
			}
		}
		request["stop_schedules"].append(item);
		result["schedules"].append(request);
	}

	std::stringstream buffer;
	buffer << this->messageID_ << "," << result << std::endl;
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

	Json::Value arrSchedules = root["schedules"];
	for(int k=0; k<arrSchedules.size(); k++){

		Json::Value itemSchedule = arrSchedules[k];
		FMOD_Schedule schedule;
		schedule.vehicleId = boost::lexical_cast<int>(itemSchedule["vehicle_id"].asCString());
		schedule.scheduleId = boost::lexical_cast<int>(itemSchedule["schedule_id"].asString());
		schedule.serviceType = itemSchedule["service_type"].asDouble();
		schedule.status = itemSchedule["status"].asInt();
		Json::Value arrStops = itemSchedule["stop_schedules"];
		for(int i=0; i<arrStops.size(); i++)
		{
			Json::Value item = arrStops[i];
			FMOD_Schedule::Stop stop;
			stop.latitude = item["latitude"].asString();
			stop.longitude = item["longitude"].asString();
			stop.arrivalTime = item["arrival_time"].asString();
			stop.depatureTime = item["departure_time"].asString();
			stop.dwellTime = 0;

			Point pos;
			pos.setX(boost::lexical_cast<double>(stop.longitude));
			pos.setY(boost::lexical_cast<double>(stop.latitude));
			sim_mob::Node* node = sim_mob::RoadNetwork::getInstance()->locateNearestNode(pos);
			schedule.routes.push_back(node);

			Json::Value arrPassengers = item["board_passengers"];
			for(int k=0; k<arrPassengers.size(); k++){
				Json::Value val = arrPassengers[k];
				stop.boardingPassengers.push_back(boost::lexical_cast<int>(val.asString()));
			}
			arrPassengers = item["alight_passengers"];
			for(int k=0; k<arrPassengers.size(); k++){
				Json::Value val = arrPassengers[k];
				stop.alightingPassengers.push_back(boost::lexical_cast<int>(val.asString()));
			}
			schedule.stopSchdules.push_back(stop);
		}

		Json::Value arrPassengers = itemSchedule["passengers"];
		for(int i=0; i<arrPassengers.size(); i++)
		{
			Json::Value item = arrPassengers[i];
			FMOD_Schedule::Passenger pass;
			pass.clientId = item["client_id"].asString();
			pass.price = item["fare"].asDouble();
			schedule.passengers.push_back(pass);
		}

		Json::Value arrRoutes = itemSchedule["route"];
		for(int i=0; i<arrRoutes.size(); i++)
		{
			Json::Value item = arrRoutes[i];
			FMOD_Schedule::Route route;
			route.latitude = item["latitude"].asString();
			route.longitude = item["longitude"].asString();
			const StreetDirectory& stdir = StreetDirectory::Instance();
			sim_mob::Node* node = nullptr;//const_cast<sim_mob::Node*>(stdir.getNode(id));
			if(node){
				schedule.routes.push_back(node);
			}
			else {
				std::cout<<"FMOD schedules message include some node id which do not exist. node id is "<<std::endl;
			}
		}
		schedules.push_back(schedule);
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
