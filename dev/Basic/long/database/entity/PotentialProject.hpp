/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   PotentialProject.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 31, 2014, 2:51 PM
 */
#pragma once

#include "Parcel.hpp"
#include "LandUseZone.hpp"
#include "DevelopmentTypeTemplate.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents a potential unit. 
         */
        class PotentialUnit {
        public:
            PotentialUnit(BigSerial unitTypeId = INVALID_ID,
                    double floorArea = 0, bool freehold = false);
            virtual ~PotentialUnit();
            //Getters
            BigSerial getUnitTypeId() const;
            double getFloorArea() const;
            bool isFreehold() const;
            friend std::ostream& operator<<(std::ostream& strm,
                    const PotentialUnit& data);
        private:
            BigSerial unitTypeId;
            double floorArea;
            bool freehold;
        };

        /**
         * Entity that represents an potential project to be converted 
         * into a project if developer model decides it.
         */
        class PotentialProject {
        public:
            PotentialProject(const DevelopmentTypeTemplate* devTemplate = nullptr,
                    const Parcel* parcel = nullptr, const LandUseZone* zone = nullptr);
            virtual ~PotentialProject();
            
            /**
             * Adds new unit.
             * @param unit to add.
             */
            void addUnit(const PotentialUnit& unit);

            //Getters
            const DevelopmentTypeTemplate* getDevTemplate() const;
            const Parcel* getParcel() const;
            const LandUseZone* getZone() const;
            const std::vector<PotentialUnit>& getUnits() const;
            double getCost() const;
            double getRevenue() const;
            //Setters
            void setCost(const double cost);
            void setRevenue(const double revenue);
            friend std::ostream& operator<<(std::ostream& strm,
                    const PotentialProject& data);
        private:
            const DevelopmentTypeTemplate* devTemplate;
            const Parcel* parcel;
            const LandUseZone* zone;
            std::vector<PotentialUnit> units;
            double cost;
            double revenue;
        };
    }
}