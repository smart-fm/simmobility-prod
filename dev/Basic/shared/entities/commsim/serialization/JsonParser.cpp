//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "JsonParser.hpp"

#include <boost/lexical_cast.hpp>

using namespace sim_mob;
using std::string;


sim_mob::Services::SIM_MOB_SERVICE sim_mob::JsonParser::getServiceType(std::string type)
{
	std::map<std::string, Services::SIM_MOB_SERVICE>::iterator it = Services::ServiceMap.find(type);
	if (it != Services::ServiceMap.end()) {
		return it->second;
	}

	return Services::SIMMOB_SRV_UNKNOWN;
}

bool sim_mob::JsonParser::parsePacketHeader(const std::string& input, pckt_header &output, Json::Value &root)
{
	Json::Value packet_header;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(input, root, false);
	if (not parsedSuccess) {
		Print() << "Parsing Packet Header for '" << input << "' Failed"	<< std::endl;
		return false;
	}
	int i = 0;
	if (root.isMember("PACKET_HEADER")) {
		packet_header = root["PACKET_HEADER"];
	} else {

		return false;
	}
	i += packet_header.isMember("NOF_MESSAGES") ? 8 : 0;
	if (!(
	(packet_header.isMember("NOF_MESSAGES"))
	)) {
		return false;
	}
	output.nof_msgs = packet_header["NOF_MESSAGES"].asString();
	return true;
}

bool sim_mob::JsonParser::parseMessageHeader(Json::Value & root, msg_header &output)
{
	if (!((root.isMember("SENDER")) && (root.isMember("SENDER_TYPE"))
			&& (root.isMember("MESSAGE_TYPE")))) {
		return false;
	}
	output.sender_id = root["SENDER"].asString();
	output.sender_type = root["SENDER_TYPE"].asString();
	output.msg_type = root["MESSAGE_TYPE"].asString();
	return true;
}


bool sim_mob::JsonParser::parseMessageHeader(const std::string& input, msg_header &output)
{
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(input, root, false);
	if (not parsedSuccess) {
		return false;
	}
	return parseMessageHeader(root,output);
}

bool sim_mob::JsonParser::getPacketMessages(const std::string& input, Json::Value &output)
{
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(input, root, false);
	if (not parsedSuccess) {
		return false;
	}
	if (!((root.isMember("DATA")) && (root["DATA"].isArray()))) {
		return false;
	}
	//actual job
	output = root["DATA"];
	return true;
}

//used for whoami id, type and required services(optional)
bool sim_mob::JsonParser::get_WHOAMI(std::string& input, std::string & type, std::string & ID, std::set<sim_mob::Services::SIM_MOB_SERVICE> &requiredServices)
{
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(input, root, false);
	if (not parsedSuccess) {
		return false;
	}

	if (!((root.isMember("ID")) && (root.isMember("TYPE")))) {
		WarnOut( "WHOAMI format incomplete.Parsing '" << input << "' Failed" << std::endl);
		return false;
	}
	ID = root["ID"].asString();
	type = root["TYPE"].asString();

	if (!root["REQUIRED_SERVICES"].isNull()) {
		if (root["REQUIRED_SERVICES"].isArray()) {
			const Json::Value services = root["REQUIRED_SERVICES"];
			for (unsigned int index = 0; index < services.size(); index++) {
				std::string type = services[index].asString();
				requiredServices.insert(getServiceType(type));
			}
		}
	}

	return true;
}

bool sim_mob::JsonParser::get_WHOAMI_Services(std::string& input, std::set<sim_mob::Services::SIM_MOB_SERVICE> & services)
{
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(input, root, false);
	if (!parsedSuccess) {
		return false;
	}
	if (!root.isMember("services")) {
		WarnOut( "Parsing services in [" << input << "] Failed" << std::endl);
		return false;
	}
	const Json::Value array = root["services"];
	for (unsigned int index = 0; index < array.size(); index++) {
//			getServiceType(array[index].asString());
		services.insert(getServiceType(array[index].asString()));
	}
	return true;
}

Json::Value sim_mob::JsonParser::createPacketHeader(pckt_header pHeader_)
{
	Json::Value header;
	header["NOF_MESSAGES"] = pHeader_.nof_msgs;
	return header;
}

Json::Value sim_mob::JsonParser::createMessageHeader(msg_header mHeader_)
{
	Json::Value header;
	header["SENDER"] = mHeader_.sender_id;
	header["SENDER_TYPE"] = mHeader_.sender_type;
	header["MESSAGE_TYPE"] = mHeader_.msg_type;
	header["MESSAGE_CAT"] = mHeader_.msg_cat;
	return header;
}

std::string sim_mob::JsonParser::makeWhoAreYouPacket()
{
	Json::Value whoAreYou_Packet_Header = createPacketHeader(
			pckt_header("1"));
	Json::Value whoAreYou = createMessageHeader(
			msg_header("0", "SIMMOBILITY", "WHOAREYOU", "SYS"));
	//no more fiels is needed
	Json::Value packet;
	packet["DATA"].append(whoAreYou);
	packet["PACKET_HEADER"] = whoAreYou_Packet_Header;
	Json::FastWriter writer;

	return writer.write(packet);
}

//	just conveys the tick
Json::Value sim_mob::JsonParser::makeTimeData(unsigned int tick, unsigned int elapsedMs)
{
	Json::Value time = createMessageHeader(msg_header("0", "SIMMOBILITY", "TIME_DATA", "SYS"));
	time["tick"] = tick;
	time["elapsed_ms"] = elapsedMs;
	return time;
}

std::string sim_mob::JsonParser::makeTimeDataString(unsigned int tick, unsigned int elapsedMs)
{
	Json::Value time = makeTimeData(tick, elapsedMs);
	Json::FastWriter writer;
	return writer.write(time);
}

std::string sim_mob::JsonParser::makeLocationMessageString(int x, int y)
{
	Json::Value loc = makeLocationMessage(x,y, LatLngLocation());
	Json::FastWriter writer;
	return writer.write(loc);
}

Json::Value sim_mob::JsonParser::makeLocationMessage(int x, int y, LatLngLocation projected)
{
	Json::Value loc = createMessageHeader(msg_header("0", "SIMMOBILITY", "LOCATION_DATA", "SYS"));
	loc["x"] = x;
	loc["y"] = y;
	loc["lat"] = projected.latitude;
	loc["lng"] = projected.longitude;

	return loc;
}

Json::Value sim_mob::JsonParser::makeRegionAndPathMessage(const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path)
{
	Json::Value res = createMessageHeader(msg_header("0", "SIMMOBILITY", "REGIONS_AND_PATH_DATA", "SYS"));

	//Add the set of "all regions" by ID
	{
	Json::Value allRegionsObj;
	for (std::vector<sim_mob::RoadRunnerRegion>::const_iterator it=all_regions.begin(); it!=all_regions.end(); it++) {
		//When we send all regions, we actually have to send the entire object, since RoadRunner needs the Lat/Lng coords in order
		// to do its own Region tracking.
		Json::Value regionObj;
		regionObj["id"] = boost::lexical_cast<string>(it->id);
		regionObj["vertices"] = Json::Value();
		for (std::vector<sim_mob::LatLngLocation>::const_iterator latlngIt=it->points.begin(); latlngIt!=it->points.end(); latlngIt++) {
			Json::Value latLngObj;
			latLngObj["latitude"] = latlngIt->latitude;
			latLngObj["longitude"] = latlngIt->longitude;
			regionObj["vertices"].append(latLngObj);
		}
		allRegionsObj.append(regionObj);
	}
	res["all_regions"] = allRegionsObj;
	}

	//Add the set of "path regions" by ID.
	{
	Json::Value pathRegionsObj;
	for (std::vector<sim_mob::RoadRunnerRegion>::const_iterator it=region_path.begin(); it!=region_path.end(); it++) {
		pathRegionsObj.append(boost::lexical_cast<string>(it->id));
	}
	res["region_path"] = pathRegionsObj;
	}

	return res;
}

Json::Value sim_mob::JsonParser::makeLocationArrayElement(unsigned int id, int x, int y)
{
	Json::Value loc;
	loc["ID"] = id;
	loc["x"] = x;
	loc["y"] = y;
	return loc;
}

//	static Json::Value makeAllLocationsMessage(int x, int y) {
//
//	}
//@originalMessage input
//@extractedType output
//@extractedData output
//@root output
bool sim_mob::JsonParser::getMessageTypeAndData(std::string &originalMessage, std::string &extractedType, std::string &extractedData, Json::Value &root_)
{
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(originalMessage, root, false);
	if (not parsedSuccess) {
		return false;
	}
	extractedType = root["MESSAGE_TYPE"].asString();
	//the rest of the message is actually named after the type name
	extractedData = root[extractedType.c_str()].asString();
	root_ = root;
	return true;
}
