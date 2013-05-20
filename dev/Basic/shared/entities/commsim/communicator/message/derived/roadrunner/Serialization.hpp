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
	static bool getTypeAndID(std::string& input, unsigned int & type, unsigned int & ID)
	{
		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if(not parsedSuccess)
		{
			std::cout << "Parsing -" << input << "- Failed" << std::endl;
			return false;
		}
		ID = root["ID"].asUInt();
		type =  root["Type"].asUInt();
		return true;
	}

	static bool getCapabilities(std::string& input, std::set<sim_mob::CAPABILITY> & capabilities)
	{

		Json::Value root;
		Json::Reader reader;
		bool parsedSuccess = reader.parse(input, root, false);
		if(! parsedSuccess)
		{
			std::cout << "Parsing [" << input << "] Failed" << std::endl;
			return false;
		}
		if(!root.isMember("capabilities"))
		{
			std::cout << "Parsing capabilities in [" << input << "] Failed" << std::endl;
			return false;
		}
		const Json::Value array = root["capabilities"];
		for(unsigned int index=0; index<array.size(); index++)
		{
			capabilities.insert(sim_mob::CapabilityMap[array[index].asString()]);
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


