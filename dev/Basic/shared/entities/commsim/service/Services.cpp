//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

#include "Services.hpp"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>

using std::string;

using namespace sim_mob;

std::map<string, sim_mob::Services::SIM_MOB_SERVICE> sim_mob::Services::ServiceMap =
		boost::assign::map_list_of
			("srv_location", SIMMOB_SRV_LOCATION)
			("srv_all_locations", SIMMOB_SRV_ALL_LOCATIONS)
			("srv_regions_and_path", SIMMOB_SRV_REGIONS_AND_PATH)
			;

sim_mob::Services::SIM_MOB_SERVICE sim_mob::Services::GetServiceType(std::string type)
{
	std::map<std::string, Services::SIM_MOB_SERVICE>::iterator it = Services::ServiceMap.find(type);
	if (it != Services::ServiceMap.end()) {
		return it->second;
	}

	throw std::runtime_error("Unknown service type string.");
}




