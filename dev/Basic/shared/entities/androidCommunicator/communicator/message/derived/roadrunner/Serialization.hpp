/*
 * Serialization.hpp
 *
 *  Created on: May 10, 2013
 *      Author: vahid
 */
#include <sstream>
namespace sim_mob
{
class JsonParser
{

public:
	static unsigned int getID(std::string values)
	{
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(values, root, false);
		if(not parsedSuccess)
		{
			std::cout << "Parsing -" << values << "- Failed" << std::endl;
			return 0;
		}

		return root["ID"].asUInt();
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


