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

sim_mob::Services::SIM_MOB_SERVICE sim_mob::Services::GetServiceType(std::string type)
{
	std::map<std::string, Services::SIM_MOB_SERVICE>::iterator it = Services::ServiceMap.find(type);
	if (it != Services::ServiceMap.end()) {
		return it->second;
	}

	return Services::SIMMOB_SRV_UNKNOWN;
}




