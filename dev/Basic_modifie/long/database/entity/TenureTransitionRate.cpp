//Copyright (c) 2016 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * TenureTransitionRate.cpp
 *
 *  Created on: 5 Feb 2016
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/TenureTransitionRate.hpp>

namespace sim_mob
{
	namespace long_term
	{
		TenureTransitionRate::TenureTransitionRate( BigSerial _id, string _ageGroup, string _currentStatus, string _futureStatus, double _rate )
		{
			id = _id;
			ageGroup = _ageGroup;
			currentStatus = _currentStatus;
			futureStatus = _futureStatus;
			rate = _rate;
		}

		TenureTransitionRate::~TenureTransitionRate() {}

		BigSerial TenureTransitionRate::getId() const
		{
			return id;
		}

		string TenureTransitionRate::getAgeGroup() const
		{
			return ageGroup;
		}

		string TenureTransitionRate::getCurrentStatus() const
		{
			return currentStatus;
		}

		string TenureTransitionRate::getFutureStatus() const
		{
			return futureStatus;
		}

		double TenureTransitionRate::getRate() const
		{
			return rate;
		}

		std::ostream& operator<<(std::ostream& strm, const TenureTransitionRate& data)
		{
			return strm << "{"
					<< "\"id\":\"" << data.id << "\","
					<< "\"ageGroup\":\"" << data.ageGroup << "\","
					<< "\"\"currentStatus:\"" << data.currentStatus << "\","
					<< "\"\"newStatus:\"" << data.futureStatus << "\","
					<< "\"\"rate:\"" << data.rate << "\","
					<< "}";
		}
	}
}
