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
            void setAgeOfHead(int ageOfHead);
            int getAgeOfHead() const;
            void setWorkers(int workers);
            int getWorkers() const;
            void setHousingDuration(int housingDuration);
            int getHousingDuration() const;
            void setIncome(double income);
            double getIncome() const;
            void setChildren(int children);
            int getChildren() const;
            void setSize(int size);
            int getSize() const;
            void setVehicleCategoryId(BigSerial vehicleCategoryId);
            BigSerial getVehicleCategoryId() const;
            void setEthnicityId(BigSerial ethnicityId);
            BigSerial getEthnicityId() const;
            void setUnitId(BigSerial unitId);
            BigSerial getUnitId() const;
            void setLifestyleId(BigSerial lifestyleId);
            BigSerial getLifestyleId() const;
            void setId(BigSerial id);
            BigSerial getId() const;

            /**
             * Operator to print the Household data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, 
                const Household& data);

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
