//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * services.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

#pragma once

//include publishing services that you provide in simmobility
#include <map>
#include "conf/ConfigParams.hpp"

namespace sim_mob {


/**
 * Wrapper class created to hold the otherwise-top-level enums/maps in Services.hpp.
 * This class is only named Services as it is the name of the file. Feel free to change it. ~Seth
 */
class Services {
public:
	enum SIM_MOB_SERVICE {
		SIMMOB_SRV_TIME,
		SIMMOB_SRV_LOCATION,
		SIMMOB_SRV_ALL_LOCATIONS,
		SIMMOB_SRV_UNKNOWN,
	};

	static std::map<std::string, SIM_MOB_SERVICE> ServiceMap;
	static std::map<std::string, ConfigParams::ClientType>	ClientTypeMap;
};


struct msg_header {
	//data
	std::string sender_id, sender_type, msg_type, msg_cat;

	//constructor
	msg_header();
	msg_header(std::string sender_id_, std::string sender_type_, std::string msg_type_, std::string msg_cat_="UNK");
};

struct pckt_header {
	//data
	std::string sender_id,sender_type, nof_msgs, size_bytes;

	//constructor(s)
	pckt_header();
	pckt_header(std::string	nof_msgs_);
	pckt_header(int	nof_msgs_);
};

}


