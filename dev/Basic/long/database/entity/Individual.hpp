//Copyright (c) 2014 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//license.txt   (http://opensource.org/licenses/MIT)

/*
 * Individual.h
 *
 *  Created on: 3 Sep, 2014
 *  Author: Chetan Rogbeer <chetan.rogbeer@smart.mit.edu>
 */

#pragma once
#include <ctime>

#include "boost/unordered_map.hpp"
#include "Common.hpp"
#include "Types.hpp"


namespace sim_mob
{
	namespace long_term
	{
		class PrimarySchool;
		class Individual
		{
		public:
			Individual(BigSerial id = INVALID_ID, BigSerial individualTypeId = INVALID_ID, BigSerial householdId = INVALID_ID, BigSerial jobId = INVALID_ID,
					   BigSerial ethnicityId = INVALID_ID, BigSerial employmentStatusId = INVALID_ID, BigSerial genderId = INVALID_ID,
					   BigSerial educationId = INVALID_ID, BigSerial occupationId = INVALID_ID, BigSerial vehicleCategoryId = INVALID_ID,
					   BigSerial transitCategoryId = INVALID_ID, BigSerial ageCategoryId = INVALID_ID, BigSerial residentialStatusId = INVALID_ID,
					   bool householdHead = false, float income = false, int memberId = 0, bool workerAtHome = false,bool carLicence = false, bool motorLicence = false, bool vanbusLicence = false,
					   std::tm dateOfBirth = std::tm(),	BigSerial studentId = 0,BigSerial industryId = 0,BigSerial ageDetailedCategory = 0,	int taxiDriver = 0,	int fixed_workplace = 0,int fixed_hours = 0);
			virtual ~Individual();

			BigSerial getId() const;
			BigSerial getIndividualTypeId() const;
			BigSerial getHouseholdId() const;
			BigSerial getJobId() const;
			BigSerial getEthnicityId() const;
			BigSerial getEmploymentStatusId() const;
			BigSerial getGenderId() const;
			BigSerial getEducationId() const;
			BigSerial getOccupationId() const;
			BigSerial getVehicleCategoryId() const;
			BigSerial getTransitCategoryId() const;
			BigSerial getAgeCategoryId() const;
			BigSerial getResidentialStatusId() const;
			bool	  getHouseholdHead() const;
			float	  getIncome() const;
			int		  getMemberId() const;
			bool	  getWorkAtHome() const;
			bool	  getCarLicense() const;
			bool	  getMotorLicense() const;
			bool	  getVanBusLicense() const;
			std::tm   getDateOfBirth() const;
			bool getIsPrimarySchoolWithin5Km(BigSerial primarySchoolId) const;

			BigSerial getStudentId() const;
			BigSerial getIndustryId() const;
			BigSerial getAgeDetailedCategory() const;
			int		  getTaxiDriver() const;
			int		  getFixed_workplace() const;
			int		  getFixed_hours() const;

			void	  setDateOfBirth(std::tm);
			void addprimarySchoolIdWithin5km(BigSerial schoolId,PrimarySchool *primarySchool);

			Individual& operator=(const Individual& source);

			friend std::ostream& operator<<(std::ostream& strm, const Individual& data);

        private:
            friend class IndividualDao;

			BigSerial id;
			BigSerial individualTypeId;
			BigSerial householdId;
			BigSerial jobId;
			BigSerial ethnicityId;
			BigSerial employmentStatusId;
			BigSerial genderId;
			BigSerial educationId;
			BigSerial occupationId;
			BigSerial vehicleCategoryId;
			BigSerial transitCategoryId;
			BigSerial ageCategoryId;
			BigSerial residentialStatusId;
			bool	  householdHead;
			float	  income;
			int		  memberId;
			bool	  workAtHome;
			bool	  carLicense;
			bool	  motorLicense;
			bool	  vanbusLicense;
			std::tm	  dateOfBirth;
			boost::unordered_map<BigSerial,PrimarySchool*> primarySchoolsWithin5KmById;

			BigSerial studentId;
			BigSerial industryId;
			BigSerial ageDetailedCategory;
			int taxiDriver;
			int fixed_workplace;
			int fixed_hours;

		};
	}
}

