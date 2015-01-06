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
        class UnitHolder;

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
            	  int story_range = 0, std::string unit_status = EMPTY_STR, double floor_area = .0f, int storey = 0, double rent = .0f, std::tm sale_from_date = std::tm(),
            	  std::tm physical_from_date = std::tm(), int sale_status = 0, int physical_status = 0, int biddingMarketEntryDay = 0);
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
            std::string getUnitStatus() const;
            double getFloorArea() const;
            int getStorey() const;
            double getRent() const;
            std::tm getSaleFromDate() const;
            std::tm getPhysicalFromDate() const;
            int getSaleStatus() const;
            int getPhysicalStatus() const;

            int  getbiddingMarketEntryDay() const;
            void setbiddingMarketEntryDay( int day );
            int  getTimeOnMarket() const;
            void setTimeOnMarket(int day );

            /**
             * Operator to print the Unit data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Unit& data );
        private:
            friend class UnitDao;
            
        private:
            friend class UnitHolder;

            //from database.
            BigSerial id;
            BigSerial building_id;
            BigSerial sla_address_id;
            int unit_type;
            int storey_range;
            std::string unit_status;
            double floor_area;
            int storey; 
            double rent;
            std::tm sale_from_date;
            std::tm physical_from_date;
            int sale_status;
            int physical_status;

            //This variable denotes the day the unit went on sale.
            int biddingMarketEntryDay;
            int timeOnMarket;
        };
    }
}
