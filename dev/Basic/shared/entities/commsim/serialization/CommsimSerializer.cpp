//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "CommsimSerializer.hpp"

#include <sstream>
#include <boost/lexical_cast.hpp>

#include "geospatial/coord/CoordinateTransform.hpp"
#include "geospatial/RoadRunnerRegion.hpp"

using namespace sim_mob;
using std::string;

namespace {

//Copies of nonary messages.
const std::string IdAckMsg ="{\"msg_type\":\"id_ack\"}";

} //End un-named namespace


void sim_mob::MessageConglomerate::addMessage(const Json::Value& msg)
{
	if (NEW_BUNDLES) { throw std::runtime_error("Error, attempting to construct v0 MessageConglomerate."); }

	messages_json.push_back(msg);
	message_bases.push_back(MessageBase());
	ParseJsonMessageBase(messages_json.back(), message_bases.back());
}


void sim_mob::MessageConglomerate::addMessage(int offset, int length, const std::string& msgStr)
{
	if (!NEW_BUNDLES) { throw std::runtime_error("Error, attempting to construct v1 MessageConglomerate."); }
	if (offsets_v1.empty() && msgStr.empty()) { throw std::runtime_error("Error; msgString must be non-empty for the first message."); }

	//Save the message string, ONCE
	if (!msgStr.empty()) {
		if (!messages_v1.empty()) {
			throw std::runtime_error("Can't overwrite the message string once set.");
		}
		messages_v1 = msgStr;
	}

	//Make sure our offset is not out of bounds.
	if (offset<0 || offset+length>static_cast<int>(messages_v1.length())) {
		throw std::runtime_error("Can't add message: total length exceeds length of message string.");
	}

	//Save the offset.
	offsets_v1.push_back(std::make_pair(offset, length));

	//Now try to parse the message type.
	messages_json.push_back(Json::Value());
	message_bases.push_back(MessageBase());

	//Check the first character to determine the type (binary/json).
	const char* raw = messages_v1.c_str();
	if (static_cast<unsigned char>(raw[offset]) == 0xBB) {
		throw std::runtime_error("Base (v1) message binary format not yet supported.");
	} else if (static_cast<unsigned char>(raw[offset]) == '{') {
		Json::Reader reader;
		if (!reader.parse(&raw[offset], &raw[offset+length], messages_json.back(), false)) {
			throw std::runtime_error("Parsing JSON message base failed.");
		}

		ParseJsonMessageBase(messages_json.back(), message_bases.back());
	} else {
		throw std::runtime_error("Unable to determine v1 message format (binary or JSON).");
	}
}


void sim_mob::MessageConglomerate::ParseJsonMessageBase(const Json::Value& root, MessageBase& res)
{
	//Check the type string.
	if (!root.isMember("msg_type")) {
		throw std::runtime_error("Base message is missing required parameter 'msg_type'.");
	}
	res.msg_type = root["msg_type"].asString();
}


int sim_mob::MessageConglomerate::getCount() const
{
	if (NEW_BUNDLES) {
		return offsets_v1.size();
	} else {
		return messages_json.size();
	}
}

MessageBase sim_mob::MessageConglomerate::getBaseMessage(int msgNumber) const
{
	return message_bases.at(msgNumber);
}

const Json::Value& sim_mob::MessageConglomerate::getJsonMessage(int msgNumber) const
{
	return messages_json.at(msgNumber);
}

void sim_mob::MessageConglomerate::getRawMessage(int msgNumber, int& offset, int& length) const
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

const std::string& sim_mob::MessageConglomerate::getSenderId() const
{
	return senderId;
}

void sim_mob::MessageConglomerate::setSenderId(const std::string& id)
{
	senderId = id;
}


void sim_mob::CommsimSerializer::serialize_begin(OngoingSerialization& ongoing, const std::string& destAgId)
{
	ongoing.vHead.sendId = "0"; //SimMobility is always ID 0.
	ongoing.vHead.destId = destAgId;
	ongoing.vHead.msgLengths.clear();
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

	//Add sender ID, destID
	for (size_t i=0; i<ongoing.vHead.sendId.size(); i++) {
		vHead[v_off++] = ongoing.vHead.sendId[i];
	}
	for (size_t i=0; i<ongoing.vHead.destId.size(); i++) {
		vHead[v_off++] = ongoing.vHead.destId[i];
	}

	//Add message lengths.
	for (std::vector<unsigned int>::const_iterator it=ongoing.vHead.msgLengths.begin(); it!=ongoing.vHead.msgLengths.end(); it++) {
		vHead[v_off] = (unsigned char)(((*it)>>16)&0xFF);
		vHead[v_off+1] = (unsigned char)(((*it)>>8)&0xFF);
		vHead[v_off+2] = (unsigned char)(((*it))&0xFF);
		v_off += 3; //Note to Java developers: do the incrementation after in C++; DON'T do ++ multiple times in one line.
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
	pktHeader["send_client"] = ongoing.vHead.sendId;
	pktHeader["dest_client"] = ongoing.vHead.destId;

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
	root["header"] = pktHeader;
	root["messages"] = dataArr;
	res = JsonSingleLineWriter(!NEW_BUNDLES).write(root);

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

	//Retrieve the sender ID from the packet header.
	if (!(root.isMember("header") && root["header"].isMember("send_client"))) {
		std::cout <<"Bundle header (or part of it) not found in input: \"" << msgStr << "\"\n";
		return false;
	}
	res.setSenderId(root["header"]["send_client"].asString());

	//Retrieve the DATA section, which must be an array.
	if (!root.isMember("messages") && root["messages"].isArray()) {
		std::cout <<"A 'messages' section with correct format was not found in input: \"" <<msgStr <<"\"\n";
		return false;
	}

	//Now extract the messages one by one.
	const Json::Value& data = root["messages"];
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

	//Following this are the sendId and destId strings.
	std::stringstream idStr;
	for (int sz=0; sz<header.sendIdLen; sz++) {
		idStr <<msgStr[i++];
	}
	vHead.sendId = idStr.str();
	res.setSenderId(vHead.sendId);
	idStr.str("");
	for (int sz=0; sz<header.destIdLen; sz++) {
		idStr <<msgStr[i++];
	}
	vHead.destId = idStr.str();

	//A series of 3-byte message field lengths follows.
	for (int msgId=0; msgId<header.messageCount; msgId++) {
		int msgSize = (((unsigned char)msgStr[i])<<16) | (((unsigned char)msgStr[i+1])<<8) | ((unsigned char)msgStr[i+2]);
		i += 3; //Note to Java developers: do the incrementation after in C++; DON'T do ++ multiple times in one line.
		vHead.msgLengths.push_back(msgSize);
	}

	//At this point, we have all the information we require. The only thing left to do is update the MessageConglomerate's cache of <offset,length> pairs for each message.
	for (std::vector<unsigned int>::const_iterator it=vHead.msgLengths.begin(); it!=vHead.msgLengths.end(); it++) {
		res.addMessage(i, *it, it==vHead.msgLengths.begin()?msgStr:"");
		i += *it;
	}

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




sim_mob::IdResponseMessage sim_mob::CommsimSerializer::parseIdResponse(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::IdResponseMessage res(msg.getBaseMessage(msgNumber));

	//We are either parsing this as JSON, or as binary; version number doesn't matter in this case.
	const Json::Value& jsMsg = msg.getJsonMessage(msgNumber);
	if (!jsMsg.isNull()) {
		//Required props.
		if (!(jsMsg.isMember("token") && jsMsg.isMember("id") && jsMsg.isMember("type") && jsMsg.isMember("services") && jsMsg["services"].isArray())) {
			throw std::runtime_error("Missing or malformed required properties.");
		}

		//Set them
		res.id = jsMsg["id"].asString();
		res.token = jsMsg["token"].asString();
		res.type = jsMsg["type"].asString();
		for (unsigned int i=0; i<jsMsg["services"].size(); i++) {
			res.services.push_back(jsMsg["services"][i].asString());
		}
	} else {
		throw std::runtime_error("parse() for binary messages not yet supported.");
	}

	return res;
}


sim_mob::RerouteRequestMessage sim_mob::CommsimSerializer::parseRerouteRequest(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::RerouteRequestMessage res(msg.getBaseMessage(msgNumber));

	//We are either parsing this as JSON, or as binary; version number doesn't matter in this case.
	const Json::Value& jsMsg = msg.getJsonMessage(msgNumber);
	if (!jsMsg.isNull()) {
		if (!jsMsg.isMember("blacklisted")) {
			throw std::runtime_error("Badly formatted RerouteRequest message.");
		}

		//Save and return.
		res.blacklistRegion = jsMsg["blacklisted"].asString();
	} else {
		throw std::runtime_error("parse() for binary messages not yet supported.");
	}
	return res;
}



sim_mob::OpaqueSendMessage sim_mob::CommsimSerializer::parseOpaqueSend(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::OpaqueSendMessage res(msg.getBaseMessage(msgNumber));

	//We are either parsing this as JSON, or as binary; version number doesn't matter in this case.
	const Json::Value& jsMsg = msg.getJsonMessage(msgNumber);
	if (!jsMsg.isNull()) {
		if (!(jsMsg.isMember("from_id") && jsMsg.isMember("to_ids") && jsMsg.isMember("broadcast") && jsMsg.isMember("data") && jsMsg["to_ids"].isArray())) {
			throw std::runtime_error("Badly formatted OPAQUE_SEND message.");
		}

		//Fairly simple.
		res.fromId = jsMsg["from_id"].asString();
		res.broadcast = jsMsg["broadcast"].asBool();
		res.data = jsMsg["data"].asString();
		const Json::Value& toIds = jsMsg["to_ids"];
		for (unsigned int i=0; i<toIds.size(); i++) {
			res.toIds.push_back(toIds[i].asString());
		}

		//Fail-safe
		if (res.broadcast && !res.toIds.empty()) {
			throw std::runtime_error("Cannot call opaque_send with both \"broadcast\" as true and a non-empty toIds list.");
		}
	} else {
		throw std::runtime_error("parse() for binary messages not yet supported.");
	}
	return res;
}


sim_mob::OpaqueReceiveMessage sim_mob::CommsimSerializer::parseOpaqueReceive(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::OpaqueReceiveMessage res(msg.getBaseMessage(msgNumber));

	//We are either parsing this as JSON, or as binary; version number doesn't matter in this case.
	const Json::Value& jsMsg = msg.getJsonMessage(msgNumber);
	if (!jsMsg.isNull()) {
		if (!(jsMsg.isMember("from_id") && jsMsg.isMember("to_id") && jsMsg.isMember("data"))) {
			throw std::runtime_error("Badly formatted OPAQUE_RECEIVE message.");
		}

		//Save and return.
		res.fromId = jsMsg["from_id"].asString();
		res.toId = jsMsg["to_id"].asString();
		res.data = jsMsg["data"].asString();
	} else {
		throw std::runtime_error("parse() for binary messages not yet supported.");
	}
	return res;
}


sim_mob::RemoteLogMessage sim_mob::CommsimSerializer::parseRemoteLog(const MessageConglomerate& msg, int msgNumber)
{
	sim_mob::RemoteLogMessage res(msg.getBaseMessage(msgNumber));

	//We are either parsing this as JSON, or as binary; version number doesn't matter in this case.
	const Json::Value& jsMsg = msg.getJsonMessage(msgNumber);
	if (!jsMsg.isNull()) {
		if (!jsMsg.isMember("log_msg")) {
			throw std::runtime_error("Badly formatted RemoteLog message.");
		}

		//Save and return.
		res.logMessage = jsMsg["log_msg"].asString();
	} else {
		throw std::runtime_error("parse() for binary messages not yet supported.");
	}
	return res;
}



std::string sim_mob::CommsimSerializer::makeIdRequest(const std::string& token)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		std::stringstream res;
		res <<"{\"msg_type\":\"id_request\",\"token\":\"" <<token <<"\"}";
		return res.str();
	}
}


std::string sim_mob::CommsimSerializer::makeIdAck()
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		return IdAckMsg;
	}
}


std::string sim_mob::CommsimSerializer::makeTickedSimMob(unsigned int tick, unsigned int elapsedMs)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		std::stringstream res;
		res <<"{\"msg_type\":\"ticked_simmob\",\"tick\":" <<tick <<",\"elapsed\":" <<elapsedMs <<"}";
		return res.str();
	}
}



std::string sim_mob::CommsimSerializer::makeLocation(int x, int y, const LatLngLocation& projected)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		std::stringstream res;
		res <<"{\"msg_type\":\"location\",\"x\":" <<x <<",\"y\":" <<y
			<<",\"lat\":" <<projected.latitude <<",\"lng\":" <<projected.longitude <<"}";
		return res.str();
	}
}



std::string sim_mob::CommsimSerializer::makeRegionsAndPath(const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		std::stringstream res;
		res <<"{\"msg_type\":\"regions_and_path\",\"regions\":[";

		//Add the set of "all regions" by ID
		for (std::vector<sim_mob::RoadRunnerRegion>::const_iterator it=all_regions.begin(); it!=all_regions.end(); it++) {
			//When we send all regions, we actually have to send the entire object, since RoadRunner needs the Lat/Lng coords in order
			// to do its own Region tracking.
			res  <<"{\"id\":\"" <<it->id <<"\",\"vertices\":[";
			for (std::vector<sim_mob::LatLngLocation>::const_iterator latlngIt=it->points.begin(); latlngIt!=it->points.end(); latlngIt++) {
				res <<"{\"lat\":" <<latlngIt->latitude <<",\"lng\":" <<latlngIt->longitude <<"}" <<((latlngIt+1)!=it->points.end()?",":"");
			}
			res <<"]}" <<((it+1)!=all_regions.end()?",":"");
		}

		res <<"],\"path\":[";

		//Add the set of "path regions" by ID.
		for (std::vector<sim_mob::RoadRunnerRegion>::const_iterator it=region_path.begin(); it!=region_path.end(); it++) {
			res <<"\"" <<it->id <<"\"" <<((it+1)!=region_path.end()?",":"");
		}

		//That's it.
		res <<"]}";
		return res.str();
	}
}


std::string sim_mob::CommsimSerializer::makeNewAgents(const std::vector<unsigned int>& addAgents, const std::vector<unsigned int>& remAgents)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		std::stringstream res;
		res <<"{\"msg_type\":\"new_agents\",\"add\":[";

		//Add all "ADD" agents.
		for (std::vector<unsigned int>::const_iterator it=addAgents.begin(); it!=addAgents.end(); it++) {
			res <<"\"" <<*it <<"\"" <<((it+1)!=addAgents.end()?",":"");
		}

		//Add all "REMOVE" agents.
		for (std::vector<unsigned int>::const_iterator it=remAgents.begin(); it!=remAgents.end(); it++) {
			res <<"\"" <<*it <<"\"" <<((it+1)!=remAgents.end()?",":"");
		}

		//That's it.
		res <<"]}";
		return res.str();
	}
}



std::string sim_mob::CommsimSerializer::makeAllLocations(const std::map<unsigned int, DPoint>& allLocations)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		std::stringstream res;
		res <<"{\"msg_type\":\"all_locations\",\"locations\":[";

		//Add all "LOCATIONS"
		for (std::map<unsigned int, DPoint>::const_iterator it=allLocations.begin(); it!=allLocations.end();) {
			res <<"{\"id\":\"" <<it->first <<"\",\"x\":" <<it->second.x <<",\"y\":" <<it->second.y <<"}";
			it++;
			res <<(it!=allLocations.end()?",":"");
		}

		//That's it.
		res <<"]}";
		return res.str();
	}
}



std::string sim_mob::CommsimSerializer::makeOpaqueSend(const std::string& fromId, const std::vector<std::string>& toIds, bool broadcast, const std::string& data)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		std::stringstream res;
		res <<"{\"msg_type\":\"opaque_send\",\"from_id\":\"" <<fromId <<"\",\"broadcast\":" <<(broadcast?"true":"false")
			<<"\",\"data\":\"" <<data <<"\",\"to_ids\":[";

		//Add all "TO_IDS"
		for (std::vector<std::string>::const_iterator it=toIds.begin(); it!=toIds.end(); it++) {
			res <<"\"" <<*it <<"\"" <<((it+1)!=toIds.end()?",":"");
		}

		//That's it.
		res <<"]}";
		return res.str();
	}
}


std::string sim_mob::CommsimSerializer::makeOpaqueReceive(const std::string& fromId, const std::string& toId, const std::string& data)
{
	if (PREFER_BINARY_MESSAGES) {
		throw std::runtime_error("addX() binary format not yet supported.");
	} else {
		std::stringstream res;
		res <<"{\"msg_type\":\"opaque_receive\",\"from_id\":\"" <<fromId <<"\",\"to_id\":\"" <<toId <<"\",\"data\":\"" <<data <<"\"}";
		return res.str();
	}
}


void sim_mob::CommsimSerializer::addGeneric(OngoingSerialization& ongoing, const std::string& msg)
{
	//Just append, and hope it's formatted correctly.
	if (NEW_BUNDLES) {
		ongoing.messages <<msg;
	} else {
		//We actually need to represent a JSON vector.
		ongoing.messages <<(ongoing.messages.str().empty()?"":",") <<msg;
	}

	//Keep the header up-to-date.
	ongoing.vHead.msgLengths.push_back(msg.size());
}




///////////////////////////////////
// JsonSingleLineWriter methods.
///////////////////////////////////


sim_mob::JsonSingleLineWriter::JsonSingleLineWriter(bool appendNewline) : yamlCompatiblityEnabled_( false ), appendNewline(appendNewline)
{
}


void sim_mob::JsonSingleLineWriter::enableYAMLCompatibility()
{
   yamlCompatiblityEnabled_ = true;
}


std::string sim_mob::JsonSingleLineWriter::write(const Json::Value &root)
{
   document_.str("");
   writeValue( root );
   if (appendNewline) {    //NOTE: This is the first major difference between JsonSingleLineWriter and FastWriter.
	   document_ << "\n";
   }
   return document_.str(); //NOTE: This (using a stringstream instead of a string) is the second major difference.
}


void sim_mob::JsonSingleLineWriter::writeValue(const Json::Value &value)
{
   switch ( value.type() )
   {
   case Json::nullValue:
      document_ << "null";
      break;
   case Json::intValue:
      document_ << Json::valueToString( value.asInt() );
      break;
   case Json::uintValue:
      document_ << Json::valueToString( value.asUInt() );
      break;
   case Json::realValue:
      document_ << Json::valueToString( value.asDouble() );
      break;
   case Json::stringValue:
      document_ << Json::valueToQuotedString( value.asCString() );
      break;
   case Json::booleanValue:
      document_ << Json::valueToString( value.asBool() );
      break;
   case Json::arrayValue:
      {
         document_ << "[";
         int size = value.size();
         for ( int index =0; index < size; ++index )
         {
            if ( index > 0 )
               document_ << ",";
            writeValue( value[index] );
         }
         document_ << "]";
      }
      break;
   case Json::objectValue:
      {
         Json::Value::Members members( value.getMemberNames() );
         document_ << "{";
         for ( Json::Value::Members::iterator it = members.begin();
               it != members.end();
               ++it )
         {
            const std::string &name = *it;
            if ( it != members.begin() )
               document_ << ",";
            document_ << Json::valueToQuotedString( name.c_str() );
            document_ << (yamlCompatiblityEnabled_ ? ": " : ":");
            writeValue( value[name] );
         }
         document_ << "}";
      }
      break;
   }
}


