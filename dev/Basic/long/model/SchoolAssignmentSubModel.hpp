/*
 * SchoolAssignmentSubModel.hpp
 *
 *  Created on: 8 Mar 2016
 *      Author: gishara
 */

#pragma once

#include <Types.hpp>
#include "HM_Model.hpp"
#include "DeveloperModel.hpp"
#include "database/entity/Unit.hpp"
#include <role/impl/HouseholdSellerRole.hpp>
#include <core/LoggerAgent.hpp>
#include <core/AgentsLookup.hpp>

namespace sim_mob
{
	namespace long_term
	{
		class SchoolAssignmentSubModel
		{
		public:
			SchoolAssignmentSubModel(HM_Model *model = nullptr );

			virtual ~SchoolAssignmentSubModel();

			void assignPrimarySchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);
			void assignPreSchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);


		private:
			HM_Model* model;
			enum CoeffParamId
			{
				HOME_SCHOOL_SAME_DGP = 1, HOME_SCHOOL_SAME_TAZ, DISTANCE_TO_SCHOOL, HAS_GIFTED_PROGRAM, HAS_SAP_PROGRAM,
				CAR_TRAVEL_TIME, WORKERS_IN_HH_X_DISTANCE, PUBLIC_TRANS_TRAVEL_TIME, LOW_INCOME_HH_X_DISTANCE,HIGH_INCOME_HH_X_DISTANCE, GIFTED_PROGRAM_X_DISTANCE,
				SAP_PROGRAM_X_DISTANCE
			};
		};

	}
}


