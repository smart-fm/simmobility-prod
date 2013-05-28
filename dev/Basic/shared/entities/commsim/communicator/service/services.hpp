/*
 * services.hpp
 *
 *  Created on: May 22, 2013
 *      Author: vahid
 */

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
		SIMMOB_SRV_LOCATION
	};


std::map<std::string, SIM_MOB_SERVICE> ServiceMap = boost::assign::map_list_of("SIMMOB_SRV_TIME", SIMMOB_SRV_TIME)("SIMMOB_SRV_LOCATION", SIMMOB_SRV_LOCATION);
}
#endif /* SERVICES_HPP_ */
