/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   Property.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 3:05 PM
 */
#pragma once

#include "Common.h"
#include "Types.h"
#include "message/MessageReceiver.hpp"

namespace sim_mob {

    namespace long_term {

        class UnitHolder;

        /**
         * Represents a unit to buy/rent/hold.
         * It can be the following:
         *  - Apartment
         *  - House
         *  - Garage
         */
        class Unit {
        public:

            Unit(UnitId id, bool available,
                    float fixedCost, float distanceToCDB,
                    float size);
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
             * Gets the size of the unit.
             * @return value with size.
             */
            float GetSize() const;

            /**
             * Gets the distance to CDB.
             * @return value with distance.
             */
            float GetDistanceToCDB() const;

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
             * Gets the reservation price.
             * @return the reservation price of the unit.
             */
            float GetReservationPrice() const;

            /**
             * Sets the reservation price.
             * @param price of the new reservation price of the unit.
             */
            void SetReservationPrice(float price);

            /**
             * Gets the hedonic price.
             * @return the hedonic price of the unit.
             */
            float GetHedonicPrice() const;

            /**
             * Sets the hedonic price.
             * @param price of the new hedonic price of the unit.
             */
            void SetHedonicPrice(float price);

            /**
             * Gets the fixed cost.
             * @return the fixed cost of the unit.
             */
            float GetFixedCost() const;

            /**
             * Sets the fixed cost.
             * @param cost of the new fixed cost of the unit.
             */
            void SetFixedCost(float cost);

            /**
             * Gets the owner endpoint for communication.
             * @return owner endpoint.
             */
            UnitHolder* GetOwner();

            /**
             * Gets the weight that represents the relation between quality and price.
             * @return weight for relation between quality and price.
             */
            float GetWeightPriceQuality() const;

            /**
             * Operator to print the Unit data.  
             */
            friend ostream& operator<<(ostream& strm, const Unit& data) {
                SharedWriteLock(data.mutex);
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"reservationPrice\":\"" << data.reservationPrice << "\","
                        << "\"fixedCost\":\"" << data.fixedCost << "\","
                        << "\"hedonicPrice\":\"" << data.hedonicPrice << "\","
                        << "\"distanceToCDB\":\"" << data.distanceToCDB << "\","
                        << "\"size\":\"" << data.size << "\","
                        << "\"weightPriceQuality\":\"" << data.weightPriceQuality << "\""
                        << "}";
            }
        private:
            //TODO: FUTURE UnitDao
            /**
             * private constructor for future Dao class.
             */
            Unit();

            /**
             * Gets the owner endpoint for communication.
             * @return owner endpoint.
             */
            void SetOwner(UnitHolder* receiver);

        private:
            friend class UnitHolder;
            UnitId id;
            bool available;
            float reservationPrice;
            float fixedCost;
            float hedonicPrice;
            float distanceToCDB;
            float size;
            float weightPriceQuality;
            UnitHolder* owner;
            mutable shared_mutex mutex;
        };
    }
}