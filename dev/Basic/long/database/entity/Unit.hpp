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
                 BigSerial typeId = INVALID_ID, BigSerial postcodeId = INVALID_ID, 
                 double area = .0f, int storey = 0, double rent = .0f,
                 bool available = false);
            Unit(const Unit& source);
            virtual ~Unit();

            /**
             * An operator to allow the unit copy.
             * @param source an Unit to be copied.
             * @return The Unit after modification
             */
            Unit& operator=(const Unit& source);

            /**
             * gets the Unit unique identifier.
             * @return value with Unit identifier.
             */
            UnitId getId() const;

            /**
             * gets the Unit unique identifier.
             * @return value with Unit identifier.
             */
            BigSerial getBuildingId() const;

            /**
             * gets type identifier of the type of unit.
             * @return type identifier {@see UnitType}.
             */
            BigSerial getTypeId() const;
            
            /**
             * gets type identifier of the postcode.
             * @return type identifier {@see Postcode}.
             */
            BigSerial getPostcodeId() const;

            /**
             * gets the storey of the unit.
             * @return unit type {@see UnitType}.
             */
            int getStorey() const;

            /**
             * gets the unit Area.
             * @return unit area value.
             */
            double getFloorArea() const;

            /**
             * gets the rent value.
             * @return rent value.
             */
            double getRent() const;

            /**
             * Verifies if home is available.
             * @return true if unit is available, false otherwise.
             */
            bool isAvailable() const;

            /**
             * sets if unit is avaliable or not.
             * @param avaliable value. 
             */
            void setAvailable(bool avaliable);
            
            /**
             * @return the hedonic price.
             */
            double getHedonicPrice() const;
            
            /**
             * @return the AskingPrice price.
             */
            double getAskingPrice() const;

            /**
             * gets the owner endpoint for communication.
             * @return owner endpoint.
             */
            UnitHolder* getOwner();
            
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
                        << "\"postcodeId\":\"" << data.postcodeId << "\","
                        << "\"floorArea\":\"" << data.floorArea << "\","
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
             * sets the hedonic price
             */
            void setHedonicPrice(double hedonicPrice);

            /**
             * sets the asking price.
             */
            void setAskingPrice(double askingPrice);

            /**
             * sets the owner of the unit.
             */
            void setOwner(UnitHolder* receiver);

        private:
            friend class UnitHolder;
            //from database.
            UnitId id;
            BigSerial buildingId;
            BigSerial typeId;
            BigSerial postcodeId;
            double floorArea;
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
