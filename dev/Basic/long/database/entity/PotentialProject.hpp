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
            PotentialUnit(BigSerial unitTypeId = INVALID_ID,int numUnits = 0,double floorArea = 0, int freehold = 0, double profitPerUnit = 0.0, double demolitionCostPerUnit = 0.0);
            virtual ~PotentialUnit();

            PotentialUnit( const PotentialUnit &source);
            PotentialUnit& operator=(const PotentialUnit& source);

            //Getters
            BigSerial getUnitTypeId() const;
            double getFloorArea() const;
            void setFloorArea(double area);
            int isFreehold() const;
            int getNumUnits() const;
            void setNumUnits(int units);
            int getNumUnits();
            void setUnitTypeId(int typeId);
            void setUnitProfit(double unitProfit);
            double getUnitProfit() const;
            void setDemolitionCostPerUnit(double demolitionCost);
            double getDemolitionCostPerUnit();
            friend std::ostream& operator<<(std::ostream& strm,  const PotentialUnit& data);

        private:
            BigSerial unitTypeId;
            double floorArea;
            int freehold;
            int numUnits;
            double profitPerUnit;
            double demolitionCostPerUnit;
        };

        /**
         * Entity that represents an potential project to be converted 
         * into a project if developer model decides it.
         */
        class PotentialProject {
        public:
            PotentialProject(const DevelopmentTypeTemplate* devTemplate = nullptr,const Parcel* parcel = nullptr, BigSerial fmParcelId = INVALID_ID, std::tm simulationDate = std::tm(), double constructionCost = 0,double grossArea = 0,double tempSelectProbability = 0,double investmentReturnRatio=0,double demolitionCost = 0, double expRatio=0,int totalUnits = 0,double acquisitionCost = 0, double landValue = 0, BigSerial buildingTypeId = 0);
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
            void addTemplateUnitType(const TemplateUnitType& templateUnitType);

            void addUnits(int unitType,int numUnits);
            //Getters
            const DevelopmentTypeTemplate* getDevTemplate() const;
            const Parcel* getParcel() const;
            std::vector<PotentialUnit>& getUnits();
            double getProfit();
            double getConstructionCost() const;
            double getGrosArea() const;
            double getInvestmentReturnRatio() const;
            double getExpRatio() const;
            double getTempSelectProbability() const;
            double getDemolitionCost() const;
            int getTotalUnits();
            double getAcquisitionCost() const;
            double getLandValue() const;
            BigSerial getFmParcelId() const;
            std::tm getSimulationDate() const;
            BigSerial getBuildingTypeId() const;

            //Setters
            void setProfit(const double profit);
            void setConstructionCost(double constructionCost);
            void setGrossArea(const double grossArea);
            friend std::ostream& operator<<(std::ostream& strm,const PotentialProject& data);
            void setInvestmentReturnRatio(double inReturnRatio);
            void setExpRatio(double exRatio);
            void setTempSelectProbability(double probability);
            void setDemolitionCost(double demCost);
            void setTotalUnits (int totUnits);
            void setAcquisitionCost(double acqCost);
            void setLandValue(double landVal);
            void setSimulationDate(std::tm simDate);
            void setBuildingTypeId(BigSerial buildingType);

            std::vector<TemplateUnitType> templateUnitTypes;

        private:
            friend class PotentialProjectDao;

            std::vector<PotentialUnit> units;
            const DevelopmentTypeTemplate* devTemplate;
            const Parcel* parcel;
            std::vector<int> unitTypes;
            typedef boost::unordered_map<int,int> UnitMap;
            UnitMap unitMap;
            BigSerial fmParcelId;
            double profit;
            double constructionCost;
            double demolitionCost;
            double grossArea;
            double investmentReturnRatio;
            double expRatio;
            double tempSelectProbability;
            int totalUnits;
            double acquisitionCost;
            double landValue;
            std::tm simulationDate;
            BigSerial buildingTypeId;

        };
    }
}
