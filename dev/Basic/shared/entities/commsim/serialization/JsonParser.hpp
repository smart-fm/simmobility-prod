//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * Serialization.hpp
 *
 *  Created on: May 10, 2013
 *      Author: vahid
 */
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <set>

#include <json/json.h>

#include "entities/commsim/service/Services.hpp"
#include "geospatial/RoadRunnerRegion.hpp"
#include "logging/Log.hpp"

namespace sim_mob {

class JsonParser {
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
};

}

/*
 * **************packet structure **************************
 * note:
 * a)packets contain sections: PACKET_HEADER , DATA
 * b)DATA section is an array of messages
 * c)each message contain message meta data elements(SENDER,SENDER_TYPE,MESSAGE_TYPE) and message specific data under a section called MESSAGE_TYPE

 1-sample packets exchanged between client and server(version2, multiple messages in a single packet) :
{
    "PACKET_HEADER": {
        "NOF_MESSAGES": "2"
    },
    "DATA": [
        {
            "MESSAGE_TYPE": "TIME_DATA",
            "MESSAGE_CAT": "SYS",
            "tick": 108
        },
        {
            "MESSAGE_TYPE": "LOCATION_DATA",
            "MESSAGE_CAT": "SYS",
            "x": 37280691,
            "y": 14371911
        }
    ]
}

 2- general message structure exchanged between clients and server:

 {
 "MESSAGE_TYPE" : "type-name",
 "type-name" :
 {
 "value-name" : data
 }
 }

 //       sample :

 * packet name : whoareyou
 * sending direction: server->client

 {
    "PACKET_HEADER": {
        "NOF_MESSAGES": "1"
    },
    "DATA": [
        {
            "MESSAGE_TYPE": "WHOAREYOU",
            "MESSAGE_CAT": "SYS",
            "SENDER": "0",
            "SENDER_TYPE": "SIMMOBILITY"
        }
    ]
}

 * packet name : whoami
 * sending direction: client->server
 * DESCRIPTION:
 * 1-client types can be android emulator, ns3 etc.
 * 2-apart from adhoc messages sent by client and handled by server,
 * each client will ask server to send him specific data regularly (for example: time and location at each tick)

 {
    "PACKET_HEADER": {
        "NOF_MESSAGES": "1"
    },
    "DATA": [
        {
            "SENDER": "client-id-A",
            "SENDER_TYPE": "ANDROID_EMULATOR",
            "MESSAGE_TYPE": "WHOAMI",
            "MESSAGE_CAT": "SYS",
            "ID": "114",
            "TYPE": "ANDROID_EMULATOR",
            "REQUIRED_SERVICES": [
                "SIMMOB_SRV_TIME",
                "SIMMOB_SRV_LOCATION"
            ]
        }
    ]
}


 before continuing, please note the possible scenarios(depending on configuration settings and available implementations) :
 simmobility                or                simmobility
 /     \                                       |
 /       \                                      |
 android      ns3                                android

* packet name : UNICAST
 * sending direction:
 * usage-1) client->server 		(FROM:android TO: simMobility DESCRIPTION: android client asks simmobility to deliver the message to the other android recipients)
 * usage-2) server->client 		(FROM:simMobility TO: ns3 or android  DESCRIPTION: simmobility delegates the network transfering of messages to ns3or another android client)
{
 "MSG_TYPE" : "UNICAST",
 "MESSAGE_CAT": "APP",
 "SENDER" : "client-id-A",
 "SENDER_TYPE": "ANDROID_EMULATOR",
 "RECEIVER" : "client-id-W",
 "DATA" : "opaque-data"
 }
 Note: client-id-x are android names and may be changed to their ns3 counterpart when ns3 is involved



 * packet name : announce
 * sending direction:
 * usage-1) client->server 		(FROM:android TO: simMobility DESCRIPTION: android client asks simmobility to deliver the message to the other android recipients)
 * usage-2) server->client 		(FROM:simMobility TO: ns3  DESCRIPTION: simmobility delegates the network transfering of messages to ns3)
 {
 "MSG_TYPE" : "ANNOUNCE",
 "MESSAGE_CAT": "APP",
 "ANNOUNCE" : {
 "SENDER" : "client-id-X",
 "RECEIVER" :[
 "client-id-W",
 "client-id-Y",
 "client-id-Z"
 ],
 "DATA" : "opaque-data"
 }
 }

 * packet name : announce_received
 * sending direction:
 * usage-1)client->server (FROM:ns3 TO: simMobility.
 * DESCRIPTION: this message is sent by ns3 to simmobility when it is received by  a ns3 node from another ns3 node,
 * simmobility has to give this message to the corresponding android client)
 *
 * usage-2)server->client : (FROM:simmobility TO:android,
 * DESCRIPTION: simmobility transfers this message(which has been sent by ns3) to the corresponding android)
 {
 "MSG_TYPE" : "ANNOUNCE",
 "MESSAGE_CAT": "APP",
 "ANNOUNCE" : {
 "SENDER" : "client-id-X",
 "RECEIVER" :[
 "client-id-W"
 ],
 "DATA" : "opaque-data"
 }
 }

 ///////////NEXT VERSION of ANNOUNCE
  {
    "PACKET_HEADER": {
        "NOF_MESSAGES": "1"
    },
    "DATA": [
        {
            "MESSAGE_TYPE": "MULTICAST",
 	 	 	"MESSAGE_CAT": "APP",
            "SENDER": "client_id_x",
            "SENDER_TYPE": "ANDROID_EMULATOR",
            "ANNOUNCE_DATA" : "XXX"
        }
    ]
}

 */

