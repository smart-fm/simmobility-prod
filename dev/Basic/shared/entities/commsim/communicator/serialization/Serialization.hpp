/*
 * Serialization.hpp
 *
 *  Created on: May 10, 2013
 *      Author: vahid
 */
#pragma once
#include <sstream>
#include "entities/commsim/communicator/service/services.hpp"
#include <set>
#include <json/json.h>
namespace sim_mob
{
class JsonParser
{
public:
	//todo find a way for this hardcoding
	static sim_mob::SIM_MOB_SERVICE getServiceType(std::string type)
	{
			if(ServiceMap.find(type) == sim_mob::ServiceMap.end())
			{
				return SIMMOB_SRV_UNKNOWN;
			}
			return sim_mob::ServiceMap[type];
	}


public:
	static bool getTypeAndID(std::string& input, unsigned int & type, unsigned int & ID)
	{
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if(not parsedSuccess)
		{
			std::cout << "Parsing [   " << input << "   ] Failed" << std::endl;
			return false;
		}
		ID = root["ID"].asUInt();
		type =  root["Type"].asUInt();
		return true;
	}

	static bool getServices(std::string& input, std::set<sim_mob::SIM_MOB_SERVICE> & services)
	{

		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if(! parsedSuccess)
		{
			std::cout << "Parsing [" << input << "] Failed" << std::endl;
			return false;
		}
		if(!root.isMember("services"))
		{
			std::cout << "Parsing services in [" << input << "] Failed" << std::endl;
			return false;
		}
		const Json::Value array = root["services"];
		for(unsigned int index=0; index<array.size(); index++)
		{
			getServiceType(array[index].asString());
//			services.insert();
		}
	}

	static std::string makeWhoAreYou()
	{
		Json::Value whoAreYou;
		whoAreYou["MessageType"] = "WhoAreYou";
		Json::FastWriter writer;

		std::ostringstream out("");
		return writer.write(whoAreYou);
	}
//	just conveys the tick
	static std::string makeTimeData(unsigned int tick)
	{
		Json::Value time;
		time["MessageType"] = "TimeData";
		Json::Value breakDown;
		breakDown["tick"] = tick;
		time["TimeData"] = breakDown;
		Json::FastWriter writer;
		return writer.write(time);
	}
	static std::string makeLocationData(int x, int y)
	{

		Json::Value time;
		time["MessageType"] = "LocationData";
		Json::Value breakDown;
		breakDown["x"] = x;
		breakDown["y"] = y;
		time["LocationData"] = breakDown;
		Json::FastWriter writer;
		return writer.write(time);
	}
	//@originalMessage input
	//@extractedType output
	//@extractedData output
	//@root output
	static bool getMessageTypeAndData(std::string &originalMessage, std::string &extractedType, std::string &extractedData, Json::Value &root_)
	{
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(originalMessage, root, false);
		if(not parsedSuccess)
		{
			std::cout << "Parsing [" << originalMessage << "] Failed" << std::endl;
			return false;
		}
		extractedType = root["MessageType"].asString();
		//the rest of the message is actually named after the type name
		extractedData = root[extractedType.c_str()].asString();
		root_ = root;
		return true;
	}
};

}


/*
 * **************packet structure **************************

1- general packet structure exchanged between clients and server:

        {
            "MessageType" : "type-name",
            "type-name" :
                {
                    "value-name" : data
                }
        }

2-sample of a   packet sent from server to client or vice versa(version-1):

        {
            "MessageType" : "TimeData",
            "TimeData" :
                {
                    "tick" : 1234
                }
        }

1-sample packets exchanged between client and server(version2, multiple messages in a single packet) :
{
    "ARRAY" :
    [
        {
            "MessageType" : "TimeData",
            "TimeData" :
                {
                    "tick" : 1234
                }
        }

        ,

        {
            "MessageType":"LocationData",
            "LocationData":
                {
                    "x":10,
                    "y":12
                }
        }

    ]
}

 * packet name : whoareyou
 * sending direction: server->client
 {
    "MSG_TYPE" : "WHO_ARE_YOU",
    "WHO_ARE_YOU" : {}
}

 * packet name : whoami
 * sending direction: client->server
 * DESCRIPTION:
 * 1-client types can be android emulator, ns3 etc.
 * 2-apart from adhoc messages sent by client and handled by server,
 * each client will ask server to send him specific data regularly (for example: time and location at each tick)
 {
    "MSG_TYPE" : "WHO_AM_I",
    "WHO_AM_I" :
    {
    "CLIENT_ID" : "1",
    "CLIENT_TYPE" : "ANDROID",
    "REQUIRED_SERVICES" : [
        "TIME",
        "LOCATION"
        ]
    }
}

before continuing, please note the possible scenarios(depending on configuration settings and available implementations) :
            simmobility                or                simmobility
                /          \                                              I
              /              \                                            I
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

 */

