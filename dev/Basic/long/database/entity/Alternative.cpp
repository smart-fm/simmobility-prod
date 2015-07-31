//Copyright (c) 2015 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Alternative.cpp
 *
 *  Created on: 31 Jul, 2015
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#include <database/entity/Alternative.hpp>

namespace sim_mob
{
	namespace long_term
	{
		Alternative::Alternative(BigSerial id, BigSerial planAreaId, std::string planAreaName, BigSerial dwellingTypeId, std::string dwellingTypeName)
								:id(id), planAreaId(planAreaId), planAreaName(planAreaName), dwellingTypeId(dwellingTypeId), dwellingTypeName(dwellingTypeName){}

		Alternative::~Alternative() {}

		Alternative::Alternative(const Alternative& source)
		{
			this->id = source.id;
			this->planAreaId = source.planAreaId;
			this->planAreaName = source.planAreaName;
			this->dwellingTypeId = source.dwellingTypeId;
			this->dwellingTypeName = source.dwellingTypeName;
		}

		Alternative& Alternative::operator=(const Alternative& source)
		{
			this->id = source.id;
			this->planAreaId = source.planAreaId;
			this->planAreaName = source.planAreaName;
			this->dwellingTypeId = source.dwellingTypeId;
			this->dwellingTypeName = source.dwellingTypeName;

			return *this;
		}

		BigSerial Alternative::getId() const
		{
			return id;
		}

		BigSerial Alternative::getPlanAreaId() const
		{
			return planAreaId;
		}

		std::string Alternative::getPlanAreaName() const
		{
			return planAreaName;
		}

		BigSerial Alternative::getDwellingTypeId() const
		{
			return dwellingTypeId;
		}

		std::string Alternative::getDwellingTypeName() const
		{
			return dwellingTypeName;
		}


		std::ostream& operator<<(std::ostream& strm, const Alternative& data) {
					return strm << "{"
							<< "\"id\":\"" << data.id << "\","
							<< "\"planAreaId\":\"" << data.planAreaId << "\","
							<< "\"planAreaName\":\"" << data.planAreaName << "\","
							<< "\"dwellingTypeid\":\"" << data.dwellingTypeId << "\","
							<< "\"dwellingTypeName\":\"" << data.dwellingTypeName << "\","
							<< "}";
		}



	}
} /* namespace sim_mob */
