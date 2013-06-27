/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
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
            /**
             * private constructor for future Dao class.
             */
            Unit();

            Unit(UnitId id,
                    BigSerial buildingId,
                    BigSerial householdId,
                    UnitType type,
                    bool available,
                    double area,
                    int storey,
                    int lastRemodulationYear,
                    double fixedPrice,
                    double taxExempt,
                    double distanceToCBD,
                    bool hasGarage,
                    double weightPriceQuality,
                    double weightStorey,
                    double weightDistanceToCBD,
                    double weightType,
                    double weightArea,
                    double weightTaxExempt,
                    double weightYearLastRemodulation);
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
             * Gets the household id that is dwelling in the unit.
             * @return household id that is dwelling in the unit.
             */
            BigSerial GetHouseholdId() const;

            /**
             * Gets type of the unit.
             * @return unit type {@see UnitType}.
             */
            UnitType GetType() const;

            /**
             * Gets the storey of the unit.
             * @return unit type {@see UnitType}.
             */
            int GetStorey() const;

            /**
             * Gets the last remodulation year.
             * @return last remodulation year.
             */
            int GetLastRemodulationYear() const;

            /**
             * Gets the unit Area.
             * @return unit area value.
             */
            double GetArea() const;

            /**
             * Gets the unit fixed price.
             * @return unit fixed price value.
             */
            double GetFixedPrice() const;

            /**
             * Gets the unit tax exempt.
             * @return unit tax exempt value.
             */
            double GetTaxExempt() const;

            /**
             * Gets the unit hedonic price.
             * @return unit hedonic price value.
             */
            double GetHedonicPrice() const;

            /**
             * Gets the distance to the business center.
             * @param the distance to the business center.
             */
            double GetDistanceToCBD() const;

            /**
             * Tells if the unit ha garage or not.
             * @return true if as garage, false otherwise.
             */
            bool HasGarage() const;

            /**
             * Gets weight associated with the relation price/quality.
             * @return unit quality/price weight value.
             */
            double GetWeightPriceQuality() const;

            /**
             * Gets weight associated with the storey.
             * @return unit storey weight value.
             */
            double GetWeightStorey() const;

            /**
             * Gets weight associated with the distance to CBD.
             * @return unit distance to CBD weight value.
             */
            double GetWeightDistanceToCBD() const;

            /**
             * Gets weight associated with the type.
             * @return unit type weight value.
             */
            double GetWeightType() const;

            /**
             * Gets weight associated with the area.
             * @return unit area weight area.
             */
            double GetWeightArea() const;

            /**
             * Gets weight associated with the tax exempt.
             * @return unit tax exempt weight area.
             */
            double GetWeightTaxExempt() const;

            /**
             * Gets weight associated with the remodulation year.
             * @return unit remodulation year weight area.
             */
            double GetWeightYearLastRemodulation() const;

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
            double GetReservationPrice() const;

            /**
             * Sets the reservation price.
             * @param price of the new reservation price of the unit.
             */
            void SetReservationPrice(double price);

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
                        << "\"householdId\":\"" << data.householdId << "\","
                        << "\"type\":\"" << data.type << "\","
                        << "\"storey\":\"" << data.storey << "\","
                        << "\"lastRemodulationYear\":\"" << data.lastRemodulationYear << "\","
                        << "\"area\":\"" << data.area << "\","
                        << "\"fixedPrice\":\"" << data.fixedPrice << "\","
                        << "\"taxExempt\":\"" << data.taxExempt << "\","
                        << "\"distanceToCBD\":\"" << data.distanceToCBD << "\","
                        << "\"hedonicPrice\":\"" << data.hedonicPrice << "\","
                        << "\"hasGarage\":\"" << data.hasGarage << "\","
                        << "\"weightPriceQuality\":\"" << data.weightPriceQuality << "\","
                        << "\"weightStorey\":\"" << data.weightStorey << "\","
                        << "\"weightDistanceToCBD\":\"" << data.weightDistanceToCBD << "\","
                        << "\"weightType\":\"" << data.weightType << "\","
                        << "\"weightArea\":\"" << data.weightArea << "\","
                        << "\"weightTaxExempt\":\"" << data.weightTaxExempt << "\","
                        << "\"weightYearLastRemodulation\":\"" << data.weightYearLastRemodulation << "\","
                        << "\"reservationPrice\":\"" << data.reservationPrice << "\","
                        << "\"available\":\"" << data.available << "\""
                        << "}";
            }
        private:
            friend class UnitDao;

            /**
             * Gets the owner endpoint for communication.
             * @return owner endpoint.
             */
            void SetOwner(UnitHolder* receiver);

        private:
            friend class UnitHolder;
            //from database.
            UnitId id;
            BigSerial buildingId;
            BigSerial householdId;
            UnitType type;
            int storey;
            int lastRemodulationYear;
            double area;
            double fixedPrice;
            double taxExempt;
            double hedonicPrice;
            double distanceToCBD;
            bool hasGarage;
            // Weights
            double weightPriceQuality;
            double weightStorey;
            double weightDistanceToCBD;
            double weightType;
            double weightArea;
            double weightTaxExempt;
            double weightYearLastRemodulation;
            //dynamic fields
            double reservationPrice;
            bool available;
            UnitHolder* owner;
            mutable boost::shared_mutex mutex;
        };
    }
}
