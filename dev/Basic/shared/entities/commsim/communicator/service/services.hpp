/*
 * services.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */
//todo, change the name and ,may be, location of this file.
#ifndef SERVICES_HPP_
#define SERVICES_HPP_
//include publishing services that you provide in simmobility
//#include "derived/TimePublisher.hpp"
//#include "derived/LocationPublisher.hpp"
#include <boost/assign/list_of.hpp>
#include <map>
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

	enum ClientType
	{
		ANDROID_EMULATOR = 1,
		NS3_SIMULATOR = 2,
		//add your client type here
	};
	static std::map<std::string, ClientType>	ClientTypeMap =
			boost::assign::map_list_of
			("ANDROID_EMULATOR", ANDROID_EMULATOR)
			("NS3_SIMULATOR", NS3_SIMULATOR);

	struct msg_header
	{
		std::string
		sender_id
		,sender_type
		,msg_type;
	};

	struct pckt_header
	{
		std::string
		sender_id
		,sender_type
		,nof_msgs
		,size_bytes;
	};
}


#endif /* SERVICES_HPP_ */
