/*
 * PreSchool.hpp
 *
 *  Created on: 15 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class PreSchool
		{
		public:
			PreSchool(BigSerial preSchoolId = INVALID_ID, std::string preSchoolName = EMPTY_STR,BigSerial postcode = INVALID_ID, std::string preSchoolDistrict = EMPTY_STR, double centroidX = 0, double centroidY = 0 );
			virtual ~PreSchool();

			double getCentroidX() const;
			double getCentroidY() const;
			const std::string& getPreSchoolDistrict() const;
			BigSerial getPostcode() const;
			BigSerial getPreSchoolId() const;
			const std::string& getPreSchoolName() const;

			void setCentroidX(double centroidX);
			void setCentroidY(double centroidY);
			void setPreSchoolDistrict(const std::string& dgp);
			void setPostcode(BigSerial postcode);
			void setPreSchoolId(BigSerial schoolId);
			void setPreSchoolName(const std::string& schoolName);

		private:
			friend class PreSchoolDao;

			BigSerial preSchoolId;
			std::string preSchoolName;
			BigSerial postcode;
			std::string preSchoolDistrict;
			double centroidX;
			double centroidY;
		};
	}

}


