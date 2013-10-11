//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Household.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on April 22, 2013, 5:54 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class Household {
        public:
            Household();
            virtual ~Household();

            Household& operator=(const Household& source);
            void SetAgeOfHead(int ageOfHead);
            int GetAgeOfHead() const;
            void SetWorkers(int workers);
            int GetWorkers() const;
            void SetHousingDuration(int housingDuration);
            int GetHousingDuration() const;
            void SetIncome(double income);
            double GetIncome() const;
            void SetChildren(int children);
            int GetChildren() const;
            void SetSize(int size);
            int GetSize() const;
            void SetVehicleCategoryId(BigSerial vehicleCategoryId);
            BigSerial GetVehicleCategoryId() const;
            void SetEthnicityId(BigSerial ethnicityId);
            BigSerial GetEthnicityId() const;
            void SetUnitId(BigSerial unitId);
            BigSerial GetUnitId() const;
            void SetLifestyleId(BigSerial lifestyleId);
            BigSerial GetLifestyleId() const;
            void SetId(BigSerial id);
            BigSerial GetId() const;

            /**
             * Operator to print the Household data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Household& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"lifestyleId\":\"" << data.lifestyleId << "\","
                        << "\"unitId\":\"" << data.unitId << "\","
                        << "\"ethnicityId\":\"" << data.ethnicityId << "\","
                        << "\"vehicleCategoryId\":\"" << data.vehicleCategoryId << "\","
                        << "\"size\":\"" << data.size << "\","
                        << "\"children\":\"" << data.children << "\","
                        << "\"income\":\"" << data.income << "\","
                        << "\"housingDuration\":\"" << data.housingDuration << "\","
                        << "\"workers\":\"" << data.workers << "\","
                        << "\"ageOfHead\":\"" << data.ageOfHead << "\""
                        << "}";
            }

        private:
            friend class HouseholdDao;
        private:
            BigSerial id;
            BigSerial lifestyleId;
            BigSerial unitId;
            BigSerial ethnicityId;
            BigSerial vehicleCategoryId;
            int size;
            int children;
            double income;
            int housingDuration;
            int workers;
            int ageOfHead;
        };
    }
}
