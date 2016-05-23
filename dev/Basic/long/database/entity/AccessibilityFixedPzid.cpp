//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * AccessibilityFixedPzid.cpp
 *
 *  Created on: 20 Jan 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/AccessibilityFixedPzid.hpp>

namespace sim_mob
{
	namespace long_term
	{
		AccessibilityFixedPzid::AccessibilityFixedPzid(int _id, int _planningAreaId, std::string _dgp, double acc_t_mfg, double acc_t_off)
													  : id(_id), planningAreaId(_planningAreaId), dgp(_dgp), accTMfg(acc_t_mfg), accTOff(acc_t_off){}

		AccessibilityFixedPzid::~AccessibilityFixedPzid(){}

		int AccessibilityFixedPzid::getId() const
		{
			return id;
		}

		int AccessibilityFixedPzid::getPlanningAreaId() const
		{
			return planningAreaId;
		}

		std::string AccessibilityFixedPzid::getDgp() const
		{
			return dgp;
		}

		double AccessibilityFixedPzid::getAccTMfg() const
		{
			return accTMfg;
		}

		double AccessibilityFixedPzid::getAccTOff() const
		{
			return accTOff;
		}

		std::ostream& operator<<(std::ostream& strm, const AccessibilityFixedPzid& data)
		{
			return strm << "{"
						<< "\"id\":\"" << data.getId() << "\","
						<< "\"planningAreaId\":\"" << data.getPlanningAreaId() << "\","
						<< "\"dgp\":\"" << data.getDgp() << "\","
						<< "\"acc_t_mfg\":\"" << data.getAccTMfg() << "\","
						<< "\"acc_t_off\":\"" << data.getAccTOff() << "\""
						<< "}";
		}
	}
}
