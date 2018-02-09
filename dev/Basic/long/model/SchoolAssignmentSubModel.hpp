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
#include "role/impl/HouseholdSellerRole.hpp"
#include "core/LoggerAgent.hpp"
#include "core/AgentsLookup.hpp"

namespace sim_mob
{
	namespace long_term
	{
		class SchoolAssignmentSubModel
		{
		public:
			SchoolAssignmentSubModel(HM_Model *model = nullptr );

			virtual ~SchoolAssignmentSubModel();

			/*
			 * first round of assigning individuals to primary schools
			 */
			void assignPrimarySchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);

			/*
			 *  identify those schools that have over 3000 students selected (or applied for) in the first round assignment
				for each over-assigned school, sort applied students by distance from home to school and select the top 3000
			 */
			void setStudentLimitInPrimarySchool();

			/*
			 *for the rest of the students, assign them to other nearby schools (within 5km distance to home) that still have positions based on the following probability calculation:
			 P_school i= (maximum allowable enrollment – current applied students) of school i / (maximum allowable enrollment – current applied students) of all schools that within 5km
			 *
			 */
			void reAllocatePrimarySchoolStudents(BigSerial individualId);

			void assignPreSchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);

			void assignSecondarySchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);


		private:
			HM_Model* model;
			enum CoeffParamId
			{
				PRI_HOME_SCHOOL_SAME_DGP = 1, PRI_HOME_SCHOOL_SAME_TAZ, PRI_DISTANCE_TO_SCHOOL, PRI_HAS_GIFTED_PROGRAM, PRI_HAS_SAP_PROGRAM,
				PRI_CAR_TRAVEL_TIME, PRI_WORKERS_IN_HH_X_DISTANCE, PRI_PUBLIC_TRANS_TRAVEL_TIME, PRI_LOW_INCOME_HH_X_DISTANCE,PRI_HIGH_INCOME_HH_X_DISTANCE, PRI_GIFTED_PROGRAM_X_DISTANCE,
				PRI_SAP_PROGRAM_X_DISTANCE, SEC_DGP_SAME, SEC_MTZ_SAME, SEC_DISTANCE, SEC_ART_PRO, SEC_MUSIC_PRO, SEC_LANG_PRO, SEC_STU_DEN, SEC_PT_TIME, SEC_PT_TRANSFER,
				SEC_INC_LOW_DIST, SEC_INC_HIGH_DIST, SEC_DIST_EXPRESS_NO
			};
		};

	}
}


