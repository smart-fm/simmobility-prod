/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   PotentialProject.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 31, 2014, 2:51 PM
 */
#pragma once

#include <boost/unordered_map.hpp>

#include "Parcel.hpp"
#include "LandUseZone.hpp"
#include "DevelopmentTypeTemplate.hpp"
#include "TemplateUnitType.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents a potential unit. 
         */
        class PotentialUnit {
        public:
            PotentialUnit(BigSerial unitTypeId = INVALID_ID,int numUnits = 0,double floorArea = 0, int freehold = 0);
            virtual ~PotentialUnit();

            PotentialUnit( const PotentialUnit &source);
            PotentialUnit& operator=(const PotentialUnit& source);

            //Getters
            BigSerial getUnitTypeId() const;
            double getFloorArea() const;
            int isFreehold() const;
            int getNumUnits() const;
            void setNumUnits(int units);
            int getNumUnits();
            void setUnitTypeId(int typeId);
            friend std::ostream& operator<<(std::ostream& strm,  const PotentialUnit& data);
        private:
            BigSerial unitTypeId;
            double floorArea;
            int freehold;
            int numUnits;
        };

        /**
         * Entity that represents an potential project to be converted 
         * into a project if developer model decides it.
         */
        class PotentialProject {
        public:
            PotentialProject(const DevelopmentTypeTemplate* devTemplate = nullptr,const Parcel* parcel = nullptr,double constructionCost = 0,double grossArea = 0,double tempSelectProbability = 0,double investmentReturnRatio=0,double demolitionCost = 0, double expRatio=0);
            virtual ~PotentialProject();
            
            PotentialProject( const PotentialProject &source);
            PotentialProject& operator=(const PotentialProject& source);

            struct ByProfit
                {
                    bool operator ()( const PotentialProject &a, const PotentialProject &b ) const
                    {
                        return a.profit < b.profit;
                    }
                };
            /**
             * Adds new unit.
             * @param unit to add.
             */
            void addUnit(const PotentialUnit& unit);

            /**
             * Adds relevant template unit types of the given development type template for this project.
             * @param template unit type to add.
             */
            void addTemplateUnitType(TemplateUnitType* templateUnitType);

            void addUnits(int unitType,int numUnits);
            //Getters
            const DevelopmentTypeTemplate* getDevTemplate() const;
            const Parcel* getParcel() const;
            const std::vector<PotentialUnit>& getUnits() const;
            double getProfit() const;
            double getConstructionCost() const;
            double getRevenue() const;
            double getGrosArea() const;
            double getInvestmentReturnRatio() const;
            double getExpRatio() const;
            double getTempSelectProbability() const;
            double getDemolitionCost() const;

            //Setters
            void setProfit(const double profit);
            void setConstructionCost(double constructionCost);
            void setGrossArea(const double grossArea);
            friend std::ostream& operator<<(std::ostream& strm,const PotentialProject& data);
            void setInvestmentReturnRatio(double inReturnRatio);
            void setExpRatio(double exRatio);
            void setTempSelectProbability(double probability);
            void setDemolitionCost(double demCost);

            std::vector<TemplateUnitType*> templateUnitTypes;
            std::vector<PotentialUnit> units;
        private:
            const DevelopmentTypeTemplate* devTemplate;
            const Parcel* parcel;

            std::vector<int> unitTypes;
            typedef boost::unordered_map<int,int> UnitMap;
            UnitMap unitMap;
            double profit;
            double constructionCost;
            double demolitionCost;
            double grossArea;
            double investmentReturnRatio;
            double expRatio;
            double tempSelectProbability;

        };
    }
}
