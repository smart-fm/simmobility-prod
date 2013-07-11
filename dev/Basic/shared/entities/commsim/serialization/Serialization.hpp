/*
 * Serialization.hpp
 *
 *  Created on: May 10, 2013
 *      Author: vahid
 */
#pragma once
#include <sstream>
#include "entities/commsim/service/services.hpp"
#include "logging/Log.hpp"
#include <set>
#include <json/json.h>
namespace sim_mob {

class JsonParser {
public:
	//todo find a way for this hardcoding
	static sim_mob::SIM_MOB_SERVICE getServiceType(std::string type) {
//		Print() << "Inside getServiceType, input '" << type << "'" ;
		if (ServiceMap.find(type) == sim_mob::ServiceMap.end()) {
			return SIMMOB_SRV_UNKNOWN;
		}
//			Print() << "   returning output '" << sim_mob::ServiceMap[type] << "'" << std::endl;
		return sim_mob::ServiceMap[type];
	}

	static bool parsePacketHeader(std::string& input, pckt_header &output,
			Json::Value &root) {
		Json::Value packet_header;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if (not parsedSuccess) {
			std::cout << "Parsing Packet Header for '" << input << "' Failed"
					<< std::endl;
			return false;
		}
		int i = 0;
		if (root.isMember("PACKET_HEADER")) {
			packet_header = root["PACKET_HEADER"];
		} else {

			std::cout << "Packet header not found.Parsing '" << input
					<< "' Failed" << std::endl;
			return false;
		}
//		i += packet_header.isMember("SENDER") ? 2 : 0;
//		i += packet_header.isMember("SENDER_TYPE") ? 4 : 0;
		i += packet_header.isMember("NOF_MESSAGES") ? 8 : 0;
//		i += packet_header.isMember("PACKET_SIZE") ? 16 : 0;
		if (!(
//				(packet_header.isMember("SENDER"))
//				&&
//				(packet_header.isMember("SENDER_TYPE"))
//				&&
		(packet_header.isMember("NOF_MESSAGES"))
//				&&
//				(packet_header.isMember("PACKET_SIZE"))
		)) {
			std::cout << "Packet header incomplete[" << i << "].Parsing '"
					<< input << "' Failed" << std::endl;
			return false;
		}
//		output.sender_id = packet_header["SENDER"].asString();
//		output.sender_type = packet_header["SENDER_TYPE"].asString();
		output.nof_msgs = packet_header["NOF_MESSAGES"].asString();
//		output.size_bytes = packet_header["PACKET_SIZE"].asString();
		return true;
	}

	static bool parseMessageHeader(Json::Value & root, msg_header &output) {
		if (!((root.isMember("SENDER")) && (root.isMember("SENDER_TYPE"))
				&& (root.isMember("MESSAGE_TYPE")))) {
			std::cout << "Message Header incomplete. Parsing  Failed" << std::endl;
			return false;
		}
		output.sender_id = root["SENDER"].asString();
		output.sender_type = root["SENDER_TYPE"].asString();
		output.msg_type = root["MESSAGE_TYPE"].asString();
		return true;
	}

	static bool parseMessageHeader(std::string& input, msg_header &output) {
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if (not parsedSuccess) {
			std::cout << "Parsing '" << input << "' Failed" << std::endl;
			return false;
		}
		return parseMessageHeader(root,output);
//		if (!((root.isMember("SENDER")) && (root.isMember("SENDER_TYPE"))
//				&& (root.isMember("MESSAGE_TYPE")))) {
//			std::cout << "Message Header incomplete. Parsing '" << input
//					<< "' Failed" << std::endl;
//			return false;
//		}
//		output.sender_id = root["SENDER"].asString();
//		output.sender_type = root["SENDER_TYPE"].asString();
//		output.msg_type = root["MESSAGE_TYPE"].asString();
//		return true;
	}

	static bool getPacketMessages(std::string& input, Json::Value &output) {
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if (not parsedSuccess) {
			std::cout << "Parsing '" << input << "' Failed" << std::endl;
			return false;
		}
		if (!((root.isMember("DATA")) && (root["DATA"].isArray()))) {
			std::cout
					<< "A 'DATA' section with correct format was not found in the message. Parsing '"
					<< input << "' Failed" << std::endl;
			return false;
		}
		//actual job
		output = root["DATA"];
		return true;
	}

	//used for whoami id, type and required services(optional)
	static bool get_WHOAMI(std::string& input, std::string & type,
			std::string & ID,
			std::set<sim_mob::SIM_MOB_SERVICE> &requiredServices) {
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if (not parsedSuccess) {
			std::cout << "Parsing [   " << input << "   ] Failed" << std::endl;
			return false;
		}

		if (!((root.isMember("ID")) && (root.isMember("TYPE")))) {
			WarnOut( "WHOAMI format incomplete.Parsing '" << input
					<< "' Failed" << std::endl);
			return false;
		}
		ID = root["ID"].asString();
		type = root["TYPE"].asString();

		if (!root["REQUIRED_SERVICES"].isNull()) {
			if (root["REQUIRED_SERVICES"].isArray()) {
				const Json::Value services = root["REQUIRED_SERVICES"];
//				Print() << "services :'" << services.toStyledString() << "'" << std::endl;
				for (unsigned int index = 0; index < services.size(); index++) {
					std::string type = services[index].asString();
					requiredServices.insert(getServiceType(type));
				}
			}
		}

		return true;
	}

	static bool get_WHOAMI_Services(std::string& input,
			std::set<sim_mob::SIM_MOB_SERVICE> & services) {

		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if (!parsedSuccess) {
			std::cout << "Parsing [" << input << "] Failed" << std::endl;
			return false;
		}
		if (!root.isMember("services")) {
			WarnOut( "Parsing services in [" << input << "] Failed"
					<< std::endl);
			return false;
		}
		const Json::Value array = root["services"];
		for (unsigned int index = 0; index < array.size(); index++) {
//			getServiceType(array[index].asString());
			services.insert(getServiceType(array[index].asString()));
		}
	}

	static Json::Value createPacketHeader(pckt_header pHeader_) {
		Json::Value header;
		header["NOF_MESSAGES"] = pHeader_.nof_msgs;
		return header;
	}
	static Json::Value createMessageHeader(msg_header mHeader_) {
		Json::Value header;
		header["SENDER"] = mHeader_.sender_id;
		header["SENDER_TYPE"] = mHeader_.sender_type;
		header["MESSAGE_TYPE"] = mHeader_.msg_type;
		return header;
	}
	static std::string makeWhoAreYouPacket() {
		Json::Value whoAreYou_Packet_Header = createPacketHeader(
				pckt_header("1"));
		Json::Value whoAreYou = createMessageHeader(
				msg_header("0", "SIMMOBILITY", "WHOAREYOU"));
		//no more fiels is needed
		Json::Value packet;
		packet["DATA"].append(whoAreYou);
		packet["PACKET_HEADER"] = whoAreYou_Packet_Header;
		Json::FastWriter writer;

		return writer.write(packet);
	}
//	just conveys the tick
	static Json::Value makeTimeData(unsigned int tick, unsigned int elapsedMs) {
		Json::Value time = createMessageHeader(msg_header("0", "SIMMOBILITY", "TIME_DATA"));
		time["tick"] = tick;
		time["elapsed_ms"] = elapsedMs;
		return time;
	}

	static std::string makeTimeDataString(unsigned int tick, unsigned int elapsedMs) {
		Json::Value time = makeTimeData(tick, elapsedMs);
		Json::FastWriter writer;
		return writer.write(time);
	}

	static std::string makeLocationMessageString(int x, int y) {
		Json::Value loc = makeLocationMessage(x,y);
		Json::FastWriter writer;
		return writer.write(loc);
	}

	static Json::Value makeLocationMessage(int x, int y) {

		Json::Value loc = createMessageHeader(msg_header("0", "SIMMOBILITY", "LOCATION_DATA"));
		loc["x"] = x;
		loc["y"] = y;

		return loc;
	}
	static Json::Value makeLocationArrayElement(unsigned int id, int x, int y)
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
	static bool getMessageTypeAndData(std::string &originalMessage,
			std::string &extractedType, std::string &extractedData,
			Json::Value &root_) {
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(originalMessage, root, false);
		if (not parsedSuccess) {
			std::cout << "Parsing [" << originalMessage << "] Failed"
					<< std::endl;
			return false;
		}
		extractedType = root["MESSAGE_TYPE"].asString();
		//the rest of the message is actually named after the type name
		extractedData = root[extractedType.c_str()].asString();
		root_ = root;
		return true;
	}
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
            "tick": 108
        },
        {
            "MESSAGE_TYPE": "LOCATION_DATA",
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
            "SENDER": "114",
            "SENDER_TYPE": "ANDROID_EMULATOR",
            "MESSAGE_TYPE": "WHOAMI",
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

 * packet name : announce
 * sending direction:
 * usage-1) client->server 		(FROM:android TO: simMobility DESCRIPTION: android client asks simmobility to deliver the message to the other android recipients)
 * usage-2) server->client 		(FROM:simMobility TO: ns3  DESCRIPTION: simmobility delegates the network transfering of messages to ns3)
 {
 "MSG_TYPE" : "ANNOUNCE",
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
            "SENDER": "client_id_x",
            "SENDER_TYPE": "ANDROID_EMULATOR",
            "ANNOUNCE_DATA" : "XXX"
        }
    ]
}

 */

