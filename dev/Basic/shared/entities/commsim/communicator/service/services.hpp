/*
 * services.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#pragma once


//include publishing services that you provide in simmobility
//#include "derived/TimePublisher.hpp"
//#include "derived/LocationPublisher.hpp"
#include <boost/assign/list_of.hpp>
#include <map>
#include <string>
#include <sstream>

#include "conf/ConfigParams.hpp"

namespace sim_mob
{
	enum SIM_MOB_SERVICE
	{
		SIMMOB_SRV_TIME,
		SIMMOB_SRV_LOCATION,
		SIMMOB_SRV_UNKNOWN
	};
	static std::map<std::string, SIM_MOB_SERVICE> ServiceMap =
					boost::assign::map_list_of
					("SIMMOB_SRV_TIME", SIMMOB_SRV_TIME)
					("SIMMOB_SRV_LOCATION", SIMMOB_SRV_LOCATION);

//	enum ConfigParams::ClientType
//	{
//		ANDROID_EMULATOR = 1,
//		NS3_SIMULATOR = 2,
//		//add your client type here
//	};
	static std::map<std::string, ConfigParams::ClientType>	ClientTypeMap =
			boost::assign::map_list_of
			("ANDROID_EMULATOR", ConfigParams::ANDROID_EMULATOR)
			("NS3_SIMULATOR", ConfigParams::NS3_SIMULATOR);

	struct msg_header
	{
		//data
		std::string
		sender_id
		,sender_type
		,msg_type;
		//constructor
		msg_header(){}
		msg_header(
				std::string	 	sender_id_
				,std::string	sender_type_
				,std::string	msg_type_
				)
		:
			sender_id(sender_id_)
		,sender_type(sender_type_)
		,msg_type(msg_type_)

		{}

	};

	struct pckt_header
	{
		//data
		std::string
		sender_id
		,sender_type
		,nof_msgs
		,size_bytes;
		//constructor(s)
		pckt_header(){}
		pckt_header(
				/*std::string		sender_id_
				,std::string	sender_type_
				,*/std::string	nof_msgs_
//				,std::string	size_bytes_
				)
		:
		/*sender_id(sender_id_)
		,sender_type(sender_type_)
		,*/nof_msgs(nof_msgs_)
//		,size_bytes(size_bytes_)
		{}

		pckt_header(
				int	nof_msgs_
				)
		{
			std::ostringstream out;
			out << nof_msgs_;
			nof_msgs = out.str();
		}
	};
}


