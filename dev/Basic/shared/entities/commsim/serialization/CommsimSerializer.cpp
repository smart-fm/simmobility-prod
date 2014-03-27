//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CommsimSerializer.hpp"

#include <boost/lexical_cast.hpp>

#include "geospatial/coord/CoordinateTransform.hpp"
#include "geospatial/RoadRunnerRegion.hpp"

using namespace sim_mob;
using std::string;


void sim_mob::MessageConglomerate::addMessage(const Json::Value& msg)
{
	if (NEW_BUNDLES) { throw std::runtime_error("Error, attempting to construct v0 MessageConglomerate."); }

	messages_v0.push_back(msg);
}


void sim_mob::MessageConglomerate::addMessage(int offset, int length, const std::string& msgStr)
{
	if (!NEW_BUNDLES) { throw std::runtime_error("Error, attempting to construct v1 MessageConglomerate."); }
	if (offsets_v1.empty() && msgStr.empty()) { throw std::runtime_error("Error; msgString must be non-empty for the first message."); }

	offsets_v1.push_back(std::make_pair(offset, length));
	if (!msgStr.empty()) {
		messages_v1 = msgStr;
	}
}

int sim_mob::MessageConglomerate::getCount() const
{
	if (NEW_BUNDLES) {
		return offsets_v1.size();
	} else {
		return messages_v0.size();
	}
}

const Json::Value& sim_mob::MessageConglomerate::getMessage(int msgNumber) const
{
	if (NEW_BUNDLES) { throw std::runtime_error("Error, attempting to retrieve v0 MessageConglomerate."); }

	return messages_v0.at(msgNumber);
}

void sim_mob::MessageConglomerate::getMessage(int msgNumber, int& offset, int& length) const
{
	if (!NEW_BUNDLES) { throw std::runtime_error("Error, attempting to retrieve v1 MessageConglomerate [1]."); }

	offset = offsets_v1.at(msgNumber).first;
	length = offsets_v1.at(msgNumber).second;
}

const std::string& sim_mob::MessageConglomerate::getUnderlyingString() const
{
	if (!NEW_BUNDLES) { throw std::runtime_error("Error, attempting to retrieve v1 MessageConglomerate [2]."); }

	return messages_v1;
}


void sim_mob::CommsimSerializer::serialize_begin(OngoingSerialization& ongoing, const std::string& destAgId)
{
	ongoing.vHead.sendId = "0"; //SimMobility is always ID 0.
	ongoing.vHead.destId = destAgId;
}

bool sim_mob::CommsimSerializer::serialize_end(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res)
{
	return NEW_BUNDLES ? serialize_end_v1(ongoing, hRes, res) : serialize_end_v0(ongoing, hRes, res);
}


bool sim_mob::CommsimSerializer::serialize_end_v1(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res)
{
	//Precalculate the varying header length.
	const size_t varHeadSize = ongoing.vHead.msgLengths.size()*3 + ongoing.vHead.sendId.size() + ongoing.vHead.destId.size();
	unsigned char vHead[varHeadSize];
	size_t v_off = 0; //Current offset into vHead.

	//Add message lengths.
	for (std::vector<unsigned int>::const_iterator it=ongoing.vHead.msgLengths.begin(); it!=ongoing.vHead.msgLengths.end(); it++) {
		vHead[v_off] = (unsigned char)(((*it)>>16)&0xFF);
		vHead[v_off+1] = (unsigned char)(((*it)>>8)&0xFF);
		vHead[v_off+2] = (unsigned char)(((*it))&0xFF);
		v_off += 3; //Note to Java developers: do the incrementation after in C++; DON'T do ++ multiple times in one line.
	}

	//Add sender ID, destID
	for (size_t i=0; i<ongoing.vHead.sendId.size(); i++) {
		vHead[v_off++] = ongoing.vHead.sendId[i];
	}
	for (size_t i=0; i<ongoing.vHead.destId.size(); i++) {
		vHead[v_off++] = ongoing.vHead.destId[i];
	}

	//Sanity check.
	if (v_off != varHeadSize) { throw std::runtime_error("Varying header size not exact; memory corruption may have occurred."); }

	//Combine.
	res = std::string(reinterpret_cast<char*>(vHead), varHeadSize) + ongoing.messages.str();

	//Reflect changes to the bundle header.
	hRes.sendIdLen = ongoing.vHead.sendId.size();
	hRes.destIdLen = ongoing.vHead.destId.size();
	hRes.messageCount = ongoing.vHead.msgLengths.size();
	hRes.remLen = res.size();

	return true;
}


bool sim_mob::CommsimSerializer::serialize_end_v0(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res)
{
	//Build the header.
	Json::Value pktHeader;
	pktHeader["NOF_MESSAGES"] = boost::lexical_cast<std::string>(ongoing.vHead.msgLengths.size());

	//Turn the current data string into a Json array. (Inefficient, but that doesn't matter for v0)
	std::string data = "[" + ongoing.messages.str() + "]";
	Json::Value dataArr;
	Json::Reader reader;
	if (!(reader.parse(data, dataArr, false) && dataArr.isArray())) {
		std::cout <<"ERROR: data section cannot be represented as array\n";
		return false;
	}

	//Combine.
	Json::Value root;
	root["PACKET_HEADER"] = pktHeader;
	root["DATA"] = dataArr;
	res = Json::FastWriter().write(root);

	//Reflect changes to the bundle header.
	hRes.sendIdLen = ongoing.vHead.sendId.size();
	hRes.destIdLen = ongoing.vHead.destId.size();
	hRes.messageCount = ongoing.vHead.msgLengths.size();
	hRes.remLen = res.size();

	return true;
}


bool sim_mob::CommsimSerializer::deserialize(const BundleHeader& header, const std::string& msgStr, MessageConglomerate& res)
{
	return NEW_BUNDLES ? deserialize_v1(header, msgStr, res) : deserialize_v0(msgStr, res);
}


bool sim_mob::CommsimSerializer::deserialize_v0(const std::string& msgStr, MessageConglomerate& res)
{
	//Parse the message into a Json object.
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(msgStr, root, false)) {
		std::cout <<"ERROR: Parsing Packet Header for Failed\n";
		return false;
	}

	//We don't actually need any information from the packet header, but it must exist.
	if (!root.isMember("PACKET_HEADER")) {
		std::cout <<"Packet header not found in input: \"" << msgStr << "\"\n";
		return false;
	}

	//Retrieve the DATA section, which must be an array.
	if (!root.isMember("DATA") && root["DATA"].isArray()) {
		std::cout <<"A 'DATA' section with correct format was not found in input: \"" <<msgStr <<"\"\n";
		return false;
	}

	//Now extract the messages one by one.
	const Json::Value& data = root["DATA"];
	for (unsigned int i=0; i<data.size(); i++) {
		res.addMessage(data[i]);
	}
	return true;
}

bool sim_mob::CommsimSerializer::deserialize_v1(const BundleHeader& header, const std::string& msgStr, MessageConglomerate& res)
{
	//First, we need to read the variable-length header:
	VaryHeader vHead;
	int i = 0; //Keep track of our first message offset

	//A series of 3-byte message field lengths follows.
	for (int msgId=0; msgId<header.messageCount; msgId++) {
		int msgSize = (((unsigned char)msgStr[i])<<16) | (((unsigned char)msgStr[i+1])<<8) | ((unsigned char)msgStr[i+2]);
		i += 3; //Note to Java developers: do the incrementation after in C++; DON'T do ++ multiple times in one line.
		vHead.msgLengths.push_back(msgSize);
	}

	//Following this are the sendId and destId strings.
	std::stringstream idStr;
	for (int sz=0; sz<header.sendIdLen; sz++) {
		idStr <<msgStr[i++];
	}
	vHead.sendId = idStr.str();
	idStr.str("");
	for (int sz=0; sz<header.destIdLen; sz++) {
		idStr <<msgStr[i++];
	}
	vHead.destId = idStr.str();

	//At this point, we have all the information we require. The only thing left to do is update the MessageConglomerate's cache of <offset,length> pairs for each message.
	for (std::vector<unsigned int>::const_iterator it=vHead.msgLengths.begin(); it!=vHead.msgLengths.end(); it++) {
		res.addMessage(i, *it, it==vHead.msgLengths.begin()?msgStr:"");
		i += *it;
	}

	return true;
}


bool sim_mob::CommsimSerializer::deserialize_single(const BundleHeader& header, const std::string& msgStr, const std::string& expectedType, sim_mob::MessageBase& resMsg, MessageConglomerate& remConglom)
{
	if (!CommsimSerializer::deserialize(header, msgStr, remConglom)) {
		std::cout <<"Error deserializing message.\n";
		return false;
	}

	//There should only be one message.
	if (remConglom.getCount() != 1) {
		std::cout <<"Error: expected a single message (" <<expectedType <<").\n";
		return false;
	}

	//Make sure it's actually of the expected type.
	if (NEW_BUNDLES) {
		throw std::runtime_error("deserialize_single for NEW_BUNDLES not yet supported.");
	} else {
		if (!(remConglom.getMessage(0).isMember("MESSAGE_TYPE") && remConglom.getMessage(0)["MESSAGE_TYPE"] == expectedType)) {
			std::cout <<"Error: unexpected message type (or none).\n";
			return false;
		}
	}

	//Parse it into a base message; we'll deal with the custom properties on our own.
	resMsg = CommsimSerializer::parseMessageBase(remConglom, 0);

	return true;
}


bool sim_mob::CommsimSerializer::parseJSON(const std::string& input, Json::Value &output)
{
	Json::Reader reader;
	bool parsedSuccess = reader.parse(input, output, false);
	if (!parsedSuccess) {
		std::cout <<"parseJSON() failed.\n";
		return false;
	}
	return true;
}


sim_mob::MessageBase sim_mob::CommsimSerializer::parseMessageBase(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::MessageBase res;

	if (NEW_BUNDLES) {
		throw std::runtime_error("deserialize_single for NEW_BUNDLES not yet supported.");
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		//Common properties.
		if (!(jsMsg.isMember("SENDER") && jsMsg.isMember("SENDER_TYPE") && jsMsg.isMember("MESSAGE_TYPE") && jsMsg.isMember("MESSAGE_CAT"))) {
			throw std::runtime_error("Base message is missing some required parameters.");
		}

		res.sender_id = jsMsg["SENDER"].asString();
		res.sender_type = jsMsg["SENDER_TYPE"].asString();
		res.msg_type = jsMsg["MESSAGE_TYPE"].asString();
		res.msg_cat = jsMsg["MESSAGE_CAT"].asString();
	}
	return res;
}


sim_mob::AgentsInfoMessage sim_mob::CommsimSerializer::parseAgentsInfo(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::AgentsInfoMessage res(CommsimSerializer::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported.");
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		//Add?
		if (jsMsg.isMember("ADD")) {
			if (!jsMsg["ADD"].isArray()) { throw std::runtime_error("AgentsInfo ADD should be an array."); }
			const Json::Value& agents = jsMsg["ADD"];
			for (unsigned int i=0; i<agents.size(); i++) {
				res.addAgentIds.push_back(agents[i]["AGENT_ID"].asUInt());
			}
		}

		//Remove?
		if (jsMsg.isMember("REMOVE")) {
			if (!jsMsg["REMOVE"].isArray()) { throw std::runtime_error("AgentsInfo REMOVE should be an array."); }
			const Json::Value& agents = jsMsg["REMOVE"];
			for (unsigned int i=0; i<agents.size(); i++) {
				res.remAgentIds.push_back(agents[i]["AGENT_ID"].asUInt());
			}
		}
	}

	return res;
}



sim_mob::AllLocationsMessage sim_mob::CommsimSerializer::parseAllLocations(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::AllLocationsMessage res(CommsimSerializer::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported.");
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		if (!jsMsg.isMember("LOCATIONS")) { throw std::runtime_error("Badly formatted AllLocations message [1]."); }

		//Parse each location into our map.
		const Json::Value& locs = jsMsg["LOCATIONS"];
		for(unsigned int i=0; i<locs.size(); i++) {
			const Json::Value& agInf = locs[i];
			if (!(agInf.isMember("ID") && agInf.isMember("x") && agInf.isMember("y"))) {
				throw std::runtime_error("Badly formatted AllLocations message [2].");
			}

			//Retrieve per-agent data, add it.
			unsigned int agId = agInf["ID"].asUInt();
			double x = agInf["x"].asDouble();
			double y = agInf["y"].asDouble();
			res.agentLocations[agId] = DPoint(x,y);
		}
	}
	return res;
}


sim_mob::OpaqueSendMessage sim_mob::CommsimSerializer::parseOpaqueSend(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::OpaqueSendMessage res(CommsimSerializer::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported.");
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		if (!(jsMsg.isMember("FROM_ID") && jsMsg.isMember("TO_IDS") && jsMsg.isMember("BROADCAST") && jsMsg.isMember("DATA") && jsMsg["TO_IDS"].isArray())) {
			throw std::runtime_error("Badly formatted OPAQUE_SEND message.");
		}

		//Fairly simple.
		res.fromId = jsMsg["FROM_ID"].asString();
		res.broadcast = jsMsg["BROADCAST"].asBool();
		res.data = jsMsg["DATA"].asString();
		const Json::Value& toIds = jsMsg["TO_IDS"];
		for (unsigned int i=0; i<toIds.size(); i++) {
			res.toIds.push_back(toIds[i].asString());
		}

		//Fail-safe
		if (res.broadcast && !res.toIds.empty()) {
			throw std::runtime_error("Cannot call opaque_send with both \"broadcast\" as true and a non-empty toIds list.");
		}
	}
	return res;
}


sim_mob::OpaqueReceiveMessage sim_mob::CommsimSerializer::parseOpaqueReceive(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::OpaqueReceiveMessage res(CommsimSerializer::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported.");
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		if (!(jsMsg.isMember("FROM_ID") && jsMsg.isMember("TO_ID") && jsMsg.isMember("DATA"))) {
			throw std::runtime_error("Badly formatted OPAQUE_RECEIVE message.");
		}

		//Save and return.
		res.fromId = jsMsg["FROM_ID"].asString();
		res.toId = jsMsg["TO_ID"].asString();
		res.data = jsMsg["DATA"].asString();
	}
	return res;
}


sim_mob::RemoteLogMessage sim_mob::CommsimSerializer::parseRemoteLog(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::RemoteLogMessage res(CommsimSerializer::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported.");
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		if (!jsMsg.isMember("log_message")) {
			throw std::runtime_error("Badly formatted RemoteLog message.");
		}

		//Save and return.
		res.logMessage = jsMsg["log_message"].asString();
	}
	return res;
}


sim_mob::RerouteRequestMessage sim_mob::CommsimSerializer::parseRerouteRequest(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::RerouteRequestMessage res(CommsimSerializer::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported.");
	} else {
		const Json::Value& jsMsg = msg.getMessage(msgNumber);

		if (!jsMsg.isMember("blacklist_region")) {
			throw std::runtime_error("Badly formatted RerouteRequest message.");
		}

		//Save and return.
		res.blacklistRegion = jsMsg["blacklist_region"].asString();
	}
	return res;
}


sim_mob::NewClientMessage sim_mob::CommsimSerializer::parseNewClient(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::NewClientMessage res(CommsimSerializer::parseMessageBase(msg, msgNumber));

	if (NEW_BUNDLES) {
		throw std::runtime_error("parse() for NEW_BUNDLES not yet supported.");
	} else {
		return res;
	}
	return res;
}


void sim_mob::CommsimSerializer::addDefaultMessageProps(Json::Value& msg, const std::string& msgType)
{
	msg["SENDER"] = "0";
	msg["SENDER_TYPE"] = "NS3_SIMULATOR";
	msg["MESSAGE_TYPE"] = msgType;
	msg["MESSAGE_CAT"] = "SYS";
}


void sim_mob::CommsimSerializer::makeWhoAmI(OngoingSerialization& ongoing, const std::string& token)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		Json::Value res;
		addDefaultMessageProps(res, "WHOAMI");
		res["ID"] = "0";
		res["token"] = token;
		res["TYPE"] = "NS3_SIMULATOR";
		res["REQUIRED_SERVICES"].append("SIMMOB_SRV_TIME");
		res["REQUIRED_SERVICES"].append("SIMMOB_SRV_ALL_LOCATIONS");
		res["REQUIRED_SERVICES"].append("SIMMOB_SRV_UNKNOWN");

		//Now append it.
		std::string nextMsg = Json::FastWriter().write(res);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
	}
}

void sim_mob::CommsimSerializer::makeClientDone(OngoingSerialization& ongoing)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		Json::Value res;
		addDefaultMessageProps(res, "CLIENT_MESSAGES_DONE");

		//Now append it.
		std::string nextMsg = Json::FastWriter().write(res);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
	}
}

void sim_mob::CommsimSerializer::makeAgentsInfo(OngoingSerialization& ongoing, const std::vector<unsigned int>& addAgents, const std::vector<unsigned int>& remAgents)
{
	addGeneric(ongoing, makeAgentsInfo(addAgents, remAgents));
}

std::string sim_mob::CommsimSerializer::makeAgentsInfo(const std::vector<unsigned int>& addAgents, const std::vector<unsigned int>& remAgents)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		Json::Value res;
		addDefaultMessageProps(res, "AGENTS_INFO");
		res["SENDER_TYPE"] = "SIMMOBILITY"; //...but override this one.

		//Add all "ADD" agents.
		Json::Value singleAgent;
		for (std::vector<unsigned int>::const_iterator it=addAgents.begin(); it!=addAgents.end(); it++) {
			singleAgent["AGENT_ID"] = *it;
			res["ADD"].append(singleAgent);
		}

		//Add all "REMOVE" agents.
		for (std::vector<unsigned int>::const_iterator it=remAgents.begin(); it!=remAgents.end(); it++) {
			singleAgent["AGENT_ID"] = *it;
			res["REMOVE"].append(singleAgent);
		}

		//Now append it.
		return Json::FastWriter().write(res);
	}
}


void sim_mob::CommsimSerializer::makeAllLocations(OngoingSerialization& ongoing, const std::map<unsigned int, DPoint>& allLocations)
{
	addGeneric(ongoing, makeAllLocations(allLocations));
}


std::string sim_mob::CommsimSerializer::makeAllLocations(const std::map<unsigned int, DPoint>& allLocations)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		Json::Value res;
		addDefaultMessageProps(res, "ALL_LOCATIONS_DATA");
		res["SENDER_TYPE"] = "SIMMOBILITY"; //...but override this one.

		//Add all "LOCATIONS"
		Json::Value singleAgent;
		for (std::map<unsigned int, DPoint>::const_iterator it=allLocations.begin(); it!=allLocations.end(); it++) {
			singleAgent["ID"] = it->first;
			singleAgent["x"] = it->second.x;
			singleAgent["y"] = it->second.y;
			res["LOCATIONS"].append(singleAgent);
		}

		//Now append it.
		return Json::FastWriter().write(res);
	}
}


/*void sim_mob::CommsimSerializer::makeMulticast(OngoingSerialization& ongoing, unsigned int sendAgentId, const std::vector<unsigned int>& receiveAgentIds, const std::string& data)
{
	addGeneric(ongoing, makeMulticast(sendAgentId, receiveAgentIds, data));
}*/




std::string sim_mob::CommsimSerializer::makeOpaqueSend(const std::string& fromId, const std::vector<std::string>& toIds, bool broadcast, const std::string& data)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		Json::Value res;
		addDefaultMessageProps(res, "OPAQUE_SEND");
		//res["SENDER_TYPE"] = "APP"; //...but override this one.
		//res["SENDER"] = "106"; //NOTE: I think this is wrong.

		//Simple props.
		res["FROM_ID"] = fromId;
		res["BROADCAST"] = broadcast;
		res["DATA"] = data;

		//Add all "TO_IDS"
		for (std::vector<std::string>::const_iterator it=toIds.begin(); it!=toIds.end(); it++) {
			res["TO_IDS"].append(*it);
		}

		//Now append it.
		return Json::FastWriter().write(res);
	}
}


std::string sim_mob::CommsimSerializer::makeOpaqueReceive(const std::string& fromId, const std::string& toId, const std::string& data)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		Json::Value res;
		addDefaultMessageProps(res, "OPAQUE_RECEIVE");

		//Simple props.
		res["FROM_ID"] = fromId;
		res["TO_ID"] = toId;
		res["DATA"] = data;

		//Now append it.
		return Json::FastWriter().write(res);
	}
}


void sim_mob::CommsimSerializer::makeGoClient(OngoingSerialization& ongoing, const std::map<unsigned int, WFD_Group>& wfdGroups)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		//First make the single message.
		Json::Value res;
		addDefaultMessageProps(res, "GOCLIENT");

		//Custom properties.
		res["ID"] = "0";
		res["TYPE"] = "NS3_SIMULATOR";

		//Multi-group formation.
		for(std::map<unsigned int, WFD_Group>::const_iterator it=wfdGroups.begin(); it!=wfdGroups.end(); it++) {
			Json::Value clientMsg;
			clientMsg["GO"] = it->second.GO;
			for(std::vector<unsigned int>::const_iterator it2=it->second.members.begin(); it2!=it->second.members.end(); it2++) {
				clientMsg["CLIENTS"].append(*it2);
			}
			res["GROUPS"].append(clientMsg);
		}

		//Now append it.
		std::string nextMsg = Json::FastWriter().write(res);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
	}
}

std::string sim_mob::CommsimSerializer::makeReadyToReceive()
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		//First make the single message.
		Json::Value res;
		addDefaultMessageProps(res, "READY_TO_RECEIVE");

		//That's it.
		return Json::FastWriter().write(res);
	}
}

std::string sim_mob::CommsimSerializer::makeLocation(int x, int y, const LatLngLocation& projected)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		//First make the single message.
		Json::Value res;
		addDefaultMessageProps(res, "LOCATION_DATA");

		//Custom properties
		res["x"] = x;
		res["y"] = y;
		res["lat"] = projected.latitude;
		res["lng"] = projected.longitude;

		//That's it.
		return Json::FastWriter().write(res);
	}
}

std::string sim_mob::CommsimSerializer::makeRegionAndPath(const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		//First make the single message.
		Json::Value res;
		addDefaultMessageProps(res, "REGIONS_AND_PATH_DATA");

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

		//That's it.
		return Json::FastWriter().write(res);
	}
}

std::string sim_mob::CommsimSerializer::makeTimeData(unsigned int tick, unsigned int elapsedMs)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		//First make the single message.
		Json::Value res;
		addDefaultMessageProps(res, "TIME_DATA");

		//Custom properties
		res["tick"] = tick;
		res["elapsed_ms"] = elapsedMs;

		//That's it.
		return Json::FastWriter().write(res);
	}
}


void sim_mob::CommsimSerializer::addGeneric(OngoingSerialization& ongoing, const std::string& msg)
{
	//Just append, and hope it's formatted correctly.
	ongoing.messages <<msg;

	//Keep the header up-to-date.
	ongoing.vHead.msgLengths.push_back(msg.size());
}


void sim_mob::CommsimSerializer::makeUnknownJSON(OngoingSerialization& ongoing, const Json::Value& json)
{
	if (NEW_BUNDLES) {
		throw std::runtime_error("addX() for NEW_BUNDLES not yet supported.");
	} else {
		//Just append, and hope it's formatted correctly.
		std::string nextMsg = Json::FastWriter().write(json);
		ongoing.messages <<nextMsg;

		//Keep the header up-to-date.
		ongoing.vHead.msgLengths.push_back(nextMsg.size());
	}
}




/*sim_mob::Services::SIM_MOB_SERVICE sim_mob::CommsimSerializer::getServiceType(std::string type)
{
	std::map<std::string, Services::SIM_MOB_SERVICE>::iterator it = Services::ServiceMap.find(type);
	if (it != Services::ServiceMap.end()) {
		return it->second;
	}

	return Services::SIMMOB_SRV_UNKNOWN;
}

bool sim_mob::CommsimSerializer::parsePacketHeader(const std::string& input, pckt_header &output, Json::Value &root)
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

bool sim_mob::CommsimSerializer::parseMessageHeader(Json::Value & root, msg_header &output)
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


bool sim_mob::CommsimSerializer::parseMessageHeader(const std::string& input, msg_header &output)
{
	Json::Value root;
	Json::Reader reader;
	bool parsedSuccess = reader.parse(input, root, false);
	if (not parsedSuccess) {
		return false;
	}
	return parseMessageHeader(root,output);
}

bool sim_mob::CommsimSerializer::getPacketMessages(const std::string& input, Json::Value &output)
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
bool sim_mob::CommsimSerializer::get_WHOAMI(std::string& input, std::string & type, std::string & ID, std::set<sim_mob::Services::SIM_MOB_SERVICE> &requiredServices)
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

bool sim_mob::CommsimSerializer::get_WHOAMI_Services(std::string& input, std::set<sim_mob::Services::SIM_MOB_SERVICE> & services)
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

Json::Value sim_mob::CommsimSerializer::createPacketHeader(pckt_header pHeader_)
{
	Json::Value header;
	header["NOF_MESSAGES"] = pHeader_.nof_msgs;
	header["DEST_AGENT"] = pHeader_.dest_agent;
	return header;
}

Json::Value sim_mob::CommsimSerializer::createMessageHeader(msg_header mHeader_)
{
	Json::Value header;
	header["SENDER"] = mHeader_.sender_id;
	header["SENDER_TYPE"] = mHeader_.sender_type;
	header["MESSAGE_TYPE"] = mHeader_.msg_type;
	header["MESSAGE_CAT"] = mHeader_.msg_cat;
	return header;
}

std::string sim_mob::CommsimSerializer::makeWhoAreYouPacket(const std::string& token)
{
	Json::Value whoAreYou_Packet_Header = createPacketHeader(pckt_header(1, "0"));
	Json::Value whoAreYou = createMessageHeader(msg_header("0", "SIMMOBILITY", "WHOAREYOU", "SYS"));
	whoAreYou["token"] = token;
	Json::Value packet;
	packet["DATA"].append(whoAreYou);
	packet["PACKET_HEADER"] = whoAreYou_Packet_Header;
	Json::FastWriter writer;

	return writer.write(packet);
}

//	just conveys the tick
Json::Value sim_mob::CommsimSerializer::makeTimeData(unsigned int tick, unsigned int elapsedMs)
{
	Json::Value time = createMessageHeader(msg_header("0", "SIMMOBILITY", "TIME_DATA", "SYS"));
	time["tick"] = tick;
	time["elapsed_ms"] = elapsedMs;
	return time;
}

std::string sim_mob::CommsimSerializer::makeTimeDataString(unsigned int tick, unsigned int elapsedMs)
{
	Json::Value time = makeTimeData(tick, elapsedMs);
	Json::FastWriter writer;
	return writer.write(time);
}

std::string sim_mob::CommsimSerializer::makeLocationMessageString(int x, int y)
{
	Json::Value loc = makeLocationMessage(x,y, LatLngLocation());
	Json::FastWriter writer;
	return writer.write(loc);
}

Json::Value sim_mob::CommsimSerializer::makeLocationMessage(int x, int y, const LatLngLocation& projected)
{
	Json::Value loc = createMessageHeader(msg_header("0", "SIMMOBILITY", "LOCATION_DATA", "SYS"));
	loc["x"] = x;
	loc["y"] = y;
	loc["lat"] = projected.latitude;
	loc["lng"] = projected.longitude;

	return loc;
}

Json::Value sim_mob::CommsimSerializer::makeRegionAndPathMessage(const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path)
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

Json::Value sim_mob::CommsimSerializer::makeLocationArrayElement(unsigned int id, int x, int y)
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
bool sim_mob::CommsimSerializer::getMessageTypeAndData(std::string &originalMessage, std::string &extractedType, std::string &extractedData, Json::Value &root_)
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
*/
