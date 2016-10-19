//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Property.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 3:05 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"
#include "geospatial/coord/CoordinateTransform.hpp"

namespace sim_mob
{
    namespace long_term
    {
        /**
         * Represents a unit to buy/rent/hold.
         * It can be the following:
         *  - Apartment
         *  - House
         */
        class Unit
        {
        public:
            Unit( BigSerial id = INVALID_ID, BigSerial buildingId = INVALID_ID, BigSerial sla_address_id = INVALID_ID, int unit_type = INVALID_ID,
            	  int story_range = 0, int constructionStatus = 0, double floor_area = .0f, int storey = 0, double monthlyRent = .0f, std::tm sale_from_date = std::tm(),
            	  std::tm occupancyFromDate = std::tm(), int sale_status = 0, int occupancyStatus = 0, std::tm lastChangedDate = std::tm(),double totalPrice = 0,
            	  std::tm valueDate = std::tm(),int tenureStatus = 0,int biddingMarketEntryDay = 0, int timeOnMarket = 0, int timeOffMarket = 0, double lagCoefficent = 0,
				  int zoneHousingType = 0, int dwellingType = 0, bool isBTO = false, bool isStaggeredBTO = false);

            Unit( const Unit& source );

            virtual ~Unit();

            /**
             * An operator to allow the unit copy.
             * @param source an Unit to be copied.
             * @return The Unit after modification
             */
            Unit& operator=(const Unit& source);

            /**
             * Getters 
             */
            BigSerial getId() const;
            BigSerial getBuildingId() const;
            BigSerial getSlaAddressId() const;
            int getUnitType() const;
            int getStoreyRange() const;
            int getConstructionStatus() const;
            double getFloorArea() const;
            int getStorey() const;
            double getMonthlyRent() const;
            std::tm getSaleFromDate() const;
            std::tm getOccupancyFromDate() const;
            int getOccupancyFromYear() const;
            int getSaleStatus() const;
            int getOccupancyStatus() const;
            std::tm getLastChangedDate() const;
            int getTenureStatus() const;
            double getTotalPrice() const;
            const std::tm& getValueDate() const;
            void setBto(bool bto);
            void setStaggeredBto(bool bto);

            /*
             * setters
             */
            void setBuildingId(BigSerial buildingId);
            void setFloorArea(double floorArea);
            void setId(BigSerial id);
            void setOccupancyFromDate(const std::tm& physicalFromDate);
            void setOccupancyStatus(int occStatus);
            void setMonthlyRent(double monthlyRent);
            void setSaleFromDate(const std::tm& saleFromDate);
            void setSaleStatus(int saleStatus);
            void setSlaAddressId(BigSerial slaAddressId);
            void setStorey(int storey);
            void setStoreyRange(int storeyRange);
            void setConstructionStatus(int unitStatus);
            void setUnitType(int unitType);
            void setLastChangedDate(std::tm date);
            void setTenureStatus(int tenureStatus);
            void setTotalPrice(double totalPrice);
            void setValueDate(const std::tm& valueDate);
            void setZoneHousingType( int value );
            void setDwellingType( int value);

            int  getbiddingMarketEntryDay() const;
            void setbiddingMarketEntryDay( int day );
            int  getTimeOnMarket() const;
            void setTimeOnMarket(int day );
            int  getTimeOffMarket() const;
            void setTimeOffMarket(int day);
            void setLagCoefficient(double lag);
            double getLagCoefficient() const;
            int  getZoneHousingType() const;
            int getDwellingType() const;
            bool isBto() const;
            bool isStaggeredBto() const;


            /**
             * Operator to print the Unit data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Unit& data );

        private:
            friend class UnitDao;
            
        private:

            //from database.
            BigSerial id;
            BigSerial building_id;
            BigSerial sla_address_id;
            int unit_type;
            int storey_range;
            int constructionStatus;
            double floor_area;
            int storey; 
            double monthlyRent;
            std::tm sale_from_date;
            std::tm occupancyFromDate;
            int sale_status;
            int occupancyStatus;
            std::tm lastChangedDate;
            double totalPrice;
            std::tm valueDate;
            int tenureStatus;
            //This variable denotes the day the unit went on sale.
            int biddingMarketEntryDay;
            int timeOnMarket;
            int timeOffMarket;
            double lagCoefficient;
            int zoneHousingType;
            int dwellingType;
            bool isBTO;
            bool isStaggeredBTO;
        };
    }
}
