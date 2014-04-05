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

	///Retrieve the underlying message string; v1
	const std::string& getUnderlyingString() const;

	///Set the sender's ID
	void setSenderId(const std::string& id);

	///Retrieve the sender's ID.
	const std::string& getSenderId() const;

private:
	//We include a copy of the sender's ID (destination will always be 0, since MessageConglomerates are only used for received messages.
	std::string senderId;

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

//Combining/Separating multiple messages via OngoingSerializations or MessageConglomerates.
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

	//Turn a string into a Json::Value object. NOTE: This is only used in one very specific case.
	static bool parseJSON(const std::string& input, Json::Value &output);

	//Append an already serialized string to an OngoingSerialization.
	static void addGeneric(OngoingSerialization& ongoing, const std::string& msg);

//Parsing functions.
public:
	//Deserialize common properties associated with all messages.
	static MessageBase parseMessageBase(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an "id_response" message.
	static IdResponseMessage parseIdResponse(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an AGENTS_INFO message.
	static AgentsInfoMessage parseAgentsInfo(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an ALL_LOCATIONS message.
	static AllLocationsMessage parseAllLocations(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an OPAQUE_SEND message.
	static OpaqueSendMessage parseOpaqueSend(const MessageConglomerate& msg, int msgNumber);

	//Deserialize an OPAQUE_RECEIVE message.
	static OpaqueReceiveMessage parseOpaqueReceive(const MessageConglomerate& msg, int msgNumber);

	//Deserialize a REMOTE_LOG message.
	static RemoteLogMessage parseRemoteLog(const MessageConglomerate& msg, int msgNumber);

	//Deserialize a REROUTE_REQUEST message.
	static RerouteRequestMessage parseRerouteRequest(const MessageConglomerate& msg, int msgNumber);

	//Deserialize a NEW_CLIENT message.
	static NewClientMessage parseNewClient(const MessageConglomerate& msg, int msgNumber);


//Serialization messages.
public:
	//Serialize "id_request" to string.
	static std::string makeIdRequest(const std::string& token);

	//Serialize AGENTS_INFO to a string.
	static std::string makeAgentsInfo(const std::vector<unsigned int>& addAgents, const std::vector<unsigned int>& remAgents);

	//Serialize READY_TO_RECEIVE to a string.
	static std::string makeReadyToReceive();

	//Serialize READY to a string.
	static std::string makeReady();

	//Serialize ALL_LOCATIONS to a string.
	static std::string makeAllLocations(const std::map<unsigned int, DPoint>& allLocations);

	//Serialize OPAQUE_SEND to a string.
	static std::string makeOpaqueSend(const std::string& fromId, const std::vector<std::string>& toIds, bool broadcast, const std::string& data);

	//Serialize OPAQUE_RECEIVE to a string.
	static std::string makeOpaqueReceive(const std::string& fromId, const std::string& toId, const std::string& data);

	//Serialize LOCATION_DATA to a string.
	static std::string makeLocation(int x, int y, const LatLngLocation& projected);

	//Serialize REGIONS_AND_PATH_DATA to a string.
	static std::string makeRegionAndPath(const std::vector<sim_mob::RoadRunnerRegion>& all_regions, const std::vector<sim_mob::RoadRunnerRegion>& region_path);

	//Serialize TIME_DATA to a string.
	static std::string makeTimeData(unsigned int tick, unsigned int elapsedMs);


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
	//static void addDefaultMessageProps(Json::Value& msg, const std::string& msgType);
};



}


