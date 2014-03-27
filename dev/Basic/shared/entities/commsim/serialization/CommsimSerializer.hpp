//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#pragma once

#include <string>
#include <vector>

#include <json/json.h>

#include "entities/commsim/serialization/BundleVersion.hpp"
#include "entities/commsim/message/MessageBase.hpp"
#include "entities/commsim/message/Messages.hpp"

namespace sim_mob {

class RoadRunnerRegion;
class CommsimSerializer;
class LatLngLocation;

//NOTE: This is used in sm4ns3; leaving it here to allow WFD serialization. We may remove it later.
struct WFD_Group{
	int groupId;
	unsigned int GO;
	std::vector<unsigned int> members;
};


/**
 * A class which represents a full bundle of messages, in v0 or v1 format.
 * For v0, this is just a vector of Json::Value objects.
 * For v1, this is a string and a vector of <offset,length> pairs describing strings that may be
 *  Json-formatted objects, or may be binary-formatted.
 * This distinction exists because v0 messages must be parsed to JSON before they can be understood,
 *  but v1 objects cannot be parsed to JSON (if they are binary).
 * Functions to retrieve the current v0 or v1 object will throw exceptions if the wrong format is set.
 */
class MessageConglomerate {
public:
	///Used to build; v0
	///Make sure you do not call this function after saving any retrieved references
	void addMessage(const Json::Value& msg);

	///Used to build; v1
	///"msgStr" must not be empty for the very first message.
	///Make sure you do not call this function after saving any retrieved references
	void addMessage(int offset, int length, const std::string& msgStr="");

	///Get message count (both versions)
	int getCount() const;

	///Used to retrieve; v0
	const Json::Value& getMessage(int msgNumber) const;

	///Used to retrieve; v1
	void getMessage(int msgNumber, int& offset, int& length) const;

	///Retriee the underlying message string; v1
	const std::string& getUnderlyingString() const;

private:
	//v0 only requires this.
	std::vector<Json::Value> messages_v0;

	//v1 requires a bit more.
	std::string messages_v1;
	std::vector< std::pair<int, int> > offsets_v1; //<start, length>
};


/**
 * Used to serialize messages one at a time efficiently.
 * You will never have to deal with the internals of this class; basically, just do the following:
 * OngoingSerialization s;
 * serialize_begin(s);
 * makeX(params, s);
 * string res; BundleHeader hRes;
 * serialize_end(s, hRes, res);
 */
class OngoingSerialization {
public:
	OngoingSerialization() {}

	//Inefficient, but needed
	OngoingSerialization(const OngoingSerialization& other) : vHead(other.vHead) {
		messages.str(other.messages.str());
	}

private:
	VaryHeader vHead;
	std::stringstream messages;  //For v1, it's just the messages one after another. For v0, it's, e.g., "{m1},{m2},{m3}".

	friend class CommsimSerializer;
};



/**
 * This class contains methods that serialize from our Message subclasses to Json and back. It also contains methods for
 *   serializing a series of Json messages to what is currently called a "packet" (a series of messages with a header).
 * The former set of functions are named as "parseX()" and "makeX()".
 * The latter set of functions are named "serialize()" and "deserialize()", with variants for when a single message is expeted.
 */
class CommsimSerializer {
public:
	///Begin serialization of a series of messages. Call this once, followed by several calls to makeX(), followed by serialize_end().
	///TODO: We can improve efficiency by taking in the total message count, senderID, and destID, and partially building the varying header here.
	///      We would need to add dummy characters for the message lengths, and then overwrite them later during serialize_end().
	static void serialize_begin(OngoingSerialization& ongoing, const std::string& destAgId);

	///Finish serialization of a series of messages. See serialize_begin() for usage.
	static bool serialize_end(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res);

	//Deserialize a string containing a PACKET_HEADER and a DATA section into a vecot of JSON objects
	// representing the data section only. The PACKET_HEADER is dealt with internally.
	static bool deserialize(const BundleHeader& header, const std::string& msgStr, MessageConglomerate& res);

	//Deserialize when only a single message is expected (the "expectedType").
	//Returns the MessageBase (since that's all we know) and the full message string (as a Conglomerate with a single item) for additional item retrieval.
	static bool deserialize_single(const BundleHeader& header, const std::string& msgStr, const std::string& expectedType, MessageBase& resMsg, MessageConglomerate& remConglom);

	//Turn a string into a Json::Value object. NOTE: This is only used in one very specific case.
	static bool parseJSON(const std::string& input, Json::Value &output);

	//Deserialize common properties associated with all messages.
	static MessageBase parseMessageBase(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an AGENTS_INFO message.
	static AgentsInfoMessage parseAgentsInfo(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an ALL_LOCATIONS message.
	static AllLocationsMessage parseAllLocations(const MessageConglomerate& msg, int msgNumber);

	//Deserialize a UNICAST message.
	static UnicastMessage parseUnicast(const MessageConglomerate& msg, int msgNumber);

	//Deserialize a MULTICAST message.
	static MulticastMessage parseMulticast(const MessageConglomerate& msg, int msgNumber);

	//Deserialize a REMOTE_LOG message.
	static RemoteLogMessage parseRemoteLog(const MessageConglomerate& msg, int msgNumber);

	//Deserialize a REROUTE_REQUEST message.
	static RerouteRequestMessage parseRerouteRequest(const MessageConglomerate& msg, int msgNumber);

	//Deserialize a NEW_CLIENT message.
	static NewClientMessage parseNewClient(const MessageConglomerate& msg, int msgNumber);

	//Serialize a WHOAMI message.
	static void makeWhoAmI(OngoingSerialization& ongoing, const std::string& token);

	//Serialize a CLIENT_MESSAGES_DONE message.
	static void makeClientDone(OngoingSerialization& ongoing);

	//Serialize an AGENTS_INFO message (used in the trace runner).
	static void makeAgentsInfo(OngoingSerialization& ongoing, const std::vector<unsigned int>& addAgents, const std::vector<unsigned int>& remAgents);

	//Serialize an ALL_LOCATIONS message (used in the trace runner).
	static void makeAllLocations(OngoingSerialization& ongoing, const std::map<unsigned int, DPoint>& allLocations);

	//Serialize a MULTICAST message (used in the trace runner).
	//(The actual client simply mutates the incoming MULTICAST message, so this function is only used in trace.)
	static void makeMulticast(OngoingSerialization& ongoing, unsigned int sendAgentId, const std::vector<unsigned int>& receiveAgentIds, const std::string& data);

	//Serialize a GOCLIENT message.
	static void makeGoClient(OngoingSerialization& ongoing, const std::map<unsigned int, WFD_Group>& wfdGroups);

	//Serialize an unknown JSON-encoded message.
	//TODO: This function will be removed soon, it is the only incompatibility currently left between v0 and v1.
	static void makeUnknownJSON(OngoingSerialization& ongoing, const Json::Value& json);


//NOTE: Alternative, string-returning functions.
public:
	//Serialize AGENTS_INFO to a string.
	//TODO: You will have to manage the OngoingSerialization struct yourself if you use this.
	static std::string makeAgentsInfo(const std::vector<unsigned int>& addAgents, const std::vector<unsigned int>& remAgents);

	//Serialize READY_TO_RECEIVE to a string.
	static std::string makeReadyToReceive();

	//Serialize ALL_LOCATIONS to a string.
	static std::string makeAllLocations(const std::map<unsigned int, DPoint>& allLocations);

	//Serialize MULTICAST to a string.
	static std::string makeMulticast(unsigned int sendAgentId, const std::vector<unsigned int>& receiveAgentIds, const std::string& data);

	//Serialize LOCATION_DATA to a string.
	static std::string makeLocation(int x, int y, const LatLngLocation& projected);

	//Serialize REGIONS_AND_PATH_DATA to a string.
	static std::string makeRegionAndPath(const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path);

	//Serialize TIME_DATA to a string.
	static std::string makeTimeData(unsigned int tick, unsigned int elapsedMs);

	//Append an already serialized string to an OngoingSerialization.
	static void addGeneric(OngoingSerialization& ongoing, const std::string& msg);


private:
	///Helper: deserialize v0
	static bool deserialize_v0(const std::string& msgStr, MessageConglomerate& res);

	///Helper: deserialize v1
	static bool deserialize_v1(const BundleHeader& header, const std::string& msgStr, MessageConglomerate& res);

	///Helper: serialize v0
	static bool serialize_end_v0(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res);

	///Helper: serialize v1
	static bool serialize_end_v1(const OngoingSerialization& ongoing, BundleHeader& hRes, std::string& res);

	//Add default MessageBase properties to an existing Json-encoded message.
	static void addDefaultMessageProps(Json::Value& msg, const std::string& msgType);
};




/*class JsonParser {
public:
	//todo find a way for this hardcoding
	static sim_mob::Services::SIM_MOB_SERVICE getServiceType(std::string type);

	static bool parsePacketHeader(const std::string& input, pckt_header &output, Json::Value &root);

	static bool parseMessageHeader(Json::Value & root, msg_header &output);

	static bool parseMessageHeader(const std::string& input, msg_header &output);

	static bool getPacketMessages(const std::string& input, Json::Value &output);

	//used for whoami id, type and required services(optional)
	static bool get_WHOAMI(std::string& input, std::string & type, std::string & ID, std::set<sim_mob::Services::SIM_MOB_SERVICE> &requiredServices);

	static bool get_WHOAMI_Services(std::string& input, std::set<sim_mob::Services::SIM_MOB_SERVICE> & services);

	static Json::Value createPacketHeader(pckt_header pHeader_);

	static Json::Value createMessageHeader(msg_header mHeader_);

	///The token is used to link to the proper ConnectionHandler when the WHOAMI response is received.
	///So make sure it is unique per ConnectionHandler (it does not have to be unique per agent).
	static std::string makeWhoAreYouPacket(const std::string& token);

//	just conveys the tick
	static Json::Value makeTimeData(unsigned int tick, unsigned int elapsedMs);

	static std::string makeTimeDataString(unsigned int tick, unsigned int elapsedMs);

	static std::string makeLocationMessageString(int x, int y);

	///Either of the input points (x,y, projected.lat, projected.lng) can be 0, but this does not
	/// necessarily mean that they are void. (Most road networks will not have (0,0) or (0.0,0.0) as
	/// valid points though).
	static Json::Value makeLocationMessage(int x, int y, const LatLngLocation& projected);

	static Json::Value makeLocationArrayElement(unsigned int id, int x, int y);

	static Json::Value makeRegionAndPathMessage(const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path);


	//@originalMessage input
	//@extractedType output
	//@extractedData output
	//@root output
	static bool getMessageTypeAndData(std::string &originalMessage, std::string &extractedType, std::string &extractedData, Json::Value &root_);
};*/

}


