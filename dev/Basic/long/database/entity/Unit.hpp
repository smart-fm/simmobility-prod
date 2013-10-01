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
#include "message/MessageReceiver.hpp"

namespace sim_mob {

    namespace long_term {

        class UnitHolder;

        /**
         * Represents a unit to buy/rent/hold.
         * It can be the following:
         *  - Apartment
         *  - House
         */
        class Unit {
        public:
            Unit(UnitId id = INVALID_ID, BigSerial buildingId = INVALID_ID, 
                 BigSerial typeId = INVALID_ID, double area = .0f, 
                 int storey = 0, double rent = .0f, bool available = false);
            Unit(const Unit& source);
            virtual ~Unit();

            /**
             * An operator to allow the unit copy.
             * @param source an Unit to be copied.
             * @return The Unit after modification
             */
            Unit& operator=(const Unit& source);

            /**
             * Gets the Unit unique identifier.
             * @return value with Unit identifier.
             */
            UnitId GetId() const;

            /**
             * Gets the Unit unique identifier.
             * @return value with Unit identifier.
             */
            BigSerial GetBuildingId() const;

            /**
             * Gets type identifier of the unit.
             * @return type identifier {@see UnitType}.
             */
            BigSerial GetTypeId() const;

            /**
             * Gets the storey of the unit.
             * @return unit type {@see UnitType}.
             */
            int GetStorey() const;

            /**
             * Gets the unit Area.
             * @return unit area value.
             */
            double GetArea() const;

            /**
             * Gets the rent value.
             * @return rent value.
             */
            double GetRent() const;

            /**
             * Verifies if home is available.
             * @return true if unit is available, false otherwise.
             */
            bool IsAvailable() const;

            /**
             * Sets if unit is avaliable or not.
             * @param avaliable value. 
             */
            void SetAvailable(bool avaliable);
            
            /**
             * @return the hedonic price.
             */
            double GetHedonicPrice() const;
            
            /**
             * @return the AskingPrice price.
             */
            double GetAskingPrice() const;

            /**
             * Gets the owner endpoint for communication.
             * @return owner endpoint.
             */
            UnitHolder* GetOwner();
            
            /**
             * Operator to print the Unit data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const Unit& data) {
            	boost::upgrade_lock<boost::shared_mutex> up_lock(data.mutex);
            	boost::upgrade_to_unique_lock<boost::shared_mutex> lock(up_lock);
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"buildingId\":\"" << data.buildingId << "\","
                        << "\"typeId\":\"" << data.typeId << "\","
                        << "\"area\":\"" << data.area << "\","
                        << "\"storey\":\"" << data.storey << "\","
                        << "\"rent\":\"" << data.rent << "\","
                        << "\"hedonicPrice\":\"" << data.hedonicPrice << "\","
                        << "\"askingPrice\":\"" << data.askingPrice << "\","
                        << "\"available\":\"" << data.available << "\""
                        << "}";
            }
        private:
            friend class UnitDao;
            friend class HouseholdSellerRole;
            
            /**
             * Sets the hedonic price
             */
            void SetHedonicPrice(double hedonicPrice);

            /**
             * Sets the asking price.
             */
            void SetAskingPrice(double askingPrice);

            /**
             * Sets the owner of the unit.
             */
            void SetOwner(UnitHolder* receiver);

        private:
            friend class UnitHolder;
            //from database.
            UnitId id;
            BigSerial buildingId;
            BigSerial typeId;
            double area;
            int storey; 
            double rent;
            bool available;
            double hedonicPrice;
            double askingPrice;
            UnitHolder* owner;
            mutable boost::shared_mutex mutex;
        };
    }
}
