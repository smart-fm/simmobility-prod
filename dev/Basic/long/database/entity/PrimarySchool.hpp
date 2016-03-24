/*
 * PrimarySchool.hpp
 *
 *  Created on: 10 Mar 2016
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class PrimarySchool
		{
		public:
			PrimarySchool(BigSerial schoolId = INVALID_ID, BigSerial postcode = INVALID_ID, double centroidX = 0, double centroidY = 0, std::string schoolName = EMPTY_STR, int giftedProgram = false, int sapProgram = false, std::string dgp = EMPTY_STR, BigSerial tazId = INVALID_ID );
			virtual ~PrimarySchool();

			PrimarySchool(const PrimarySchool& source);
			PrimarySchool& operator=(const PrimarySchool& source);

			double getCentroidX() const;
			double getCentroidY() const;
			const std::string& getDgp() const;
			int isGiftedProgram() const;
			BigSerial getPostcode() const;
			int isSapProgram() const;
			BigSerial getSchoolId() const;
			const std::string& getSchoolName() const;
			BigSerial getTazId() const;

			void setCentroidX(double centroidX);
			void setCentroidY(double centroidY);
			void setDgp(const std::string& dgp);
			void setGiftedProgram(int giftedProgram);
			void setPostcode(BigSerial postcode);
			void setSapProgram(int sapProgram);
			void setSchoolId(BigSerial schoolId);
			void setSchoolName(const std::string& schoolName);
			void setTazId(BigSerial tazId);

		private:
			friend class PrimarySchoolDao;

			BigSerial schoolId;
			BigSerial postcode;
			double centroidX;
			double centroidY;
			std::string schoolName;
			int giftedProgram;
			int sapProgram;
			std::string dgp;
			BigSerial tazId;
		};
	}

}


