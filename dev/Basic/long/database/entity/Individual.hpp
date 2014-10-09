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

#include "Common.hpp"
#include "Types.hpp"
#include <ctime>


namespace sim_mob
{
	namespace long_term
	{
		class Individual
		{
		public:
			Individual(BigSerial id = INVALID_ID, BigSerial individualTypeId = INVALID_ID, BigSerial householdId = INVALID_ID, BigSerial jobId = INVALID_ID,
					   BigSerial ethnicityId = INVALID_ID, BigSerial employmentStatusId = INVALID_ID, BigSerial genderId = INVALID_ID,
					   BigSerial educationId = INVALID_ID, BigSerial occupationId = INVALID_ID, BigSerial vehicleCategoryId = INVALID_ID,
					   BigSerial transitCategoryId = INVALID_ID, BigSerial ageCategoryId = INVALID_ID, BigSerial residentialStatusId = INVALID_ID,
					   bool householdHead = false, float income = false, int memberId = 0, bool workerAtHome = false,bool driversLicence = false, std::tm dateOfBirth = std::tm());
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
			bool	  getDriversLicense() const;
			std::tm   getDateOfBirth() const;

			void	  setDateOfBirth(std::tm);

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
			bool	  driversLicense;
			std::tm	  dateOfBirth;


		};
	}
}

