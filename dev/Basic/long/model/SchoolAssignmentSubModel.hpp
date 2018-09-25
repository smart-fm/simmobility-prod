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

            struct DistanceEzLinkStop
            {
                BigSerial stopId;
                double distanceToSchool;
            };

            struct OrderByDistance
            {
                bool operator ()( const DistanceEzLinkStop &a, const DistanceEzLinkStop &b ) const

                {
                    return a.distanceToSchool < b.distanceToSchool;
                }
            };

            bool IsUniqueSchoolStop (StudentStop& s1, StudentStop s2) { return (s1.getSchoolStopEzLinkId()==s2.getSchoolStopEzLinkId()); }
            /*
             * first round of assigning individuals to primary schools
             */
            void assignPrimarySchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);

            void assignPreSchool( Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);

            void assignSecondarySchool(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);

            //sort the ezlink stops by the distance from student home to ezlink stop.
            void getSortedDistanceStopList(std::vector<SchoolAssignmentSubModel::DistanceEzLinkStop>& ezLinkStopsWithDistanceFromHomeToSchool);

            std::map<BigSerial, int> getStudentStopFequencyMap(HM_Model::StudentStopList &list);

            void assignUniversity(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);

            void assignPolyTechnic(const Household *household,BigSerial individualId, HouseholdAgent *hhAgent, int day);


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


