//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Services.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>

using std::string;

using namespace sim_mob;

std::map<string, sim_mob::Services::SIM_MOB_SERVICE> sim_mob::Services::ServiceMap =
		boost::assign::map_list_of
			("SIMMOB_SRV_TIME", SIMMOB_SRV_TIME)
			("SIMMOB_SRV_LOCATION", SIMMOB_SRV_LOCATION)
			("SIMMOB_SRV_ALL_LOCATIONS", SIMMOB_SRV_ALL_LOCATIONS)
			("SIMMOB_SRV_REGIONS_AND_PATH", SIMMOB_SRV_REGIONS_AND_PATH)
			;

std::map<string, comm::ClientType> sim_mob::Services::ClientTypeMap =
		boost::assign::map_list_of
			("ANDROID_EMULATOR", comm::ANDROID_EMULATOR)
			("NS3_SIMULATOR", comm::NS3_SIMULATOR);


sim_mob::msg_header::msg_header()
{}

sim_mob::msg_header::msg_header(string sender_id_, string sender_type_, string msg_type_, string msg_cat_)
	: sender_id(sender_id_), sender_type(sender_type_), msg_type(msg_type_), msg_cat(msg_cat_)
{}

sim_mob::pckt_header::pckt_header()
{}

sim_mob::pckt_header::pckt_header(string nof_msgs_)
	: nof_msgs(nof_msgs_)
{}

sim_mob::pckt_header::pckt_header(int nof_msgs_) : nof_msgs(boost::lexical_cast<string>(nof_msgs_))
{}



