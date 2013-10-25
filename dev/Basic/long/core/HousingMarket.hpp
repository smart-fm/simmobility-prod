//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   HousingMarket.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on March 11, 2013, 4:13 PM
 */
#pragma once

#include "util/UnitHolder.hpp"
#include "entities/Entity.hpp"

namespace sim_mob {

    namespace long_term {
        
        class UnitEntry {
        public:
            UnitEntry(Unit& unit, double price);
            UnitEntry(const UnitEntry& orig);
            virtual ~UnitEntry();
            UnitEntry& operator=(const UnitEntry& source);

            /**
             * Gets unit.
             * @return {@link Unit} reference.
             */
            const Unit& getUnit() const;

            /**
             * Gets the price of the unit.
             * @return {@link Bid} instance.
             */
            double getPrice() const;
            
        private:
            Unit& unit;
            double price;
        };

        /**
         * Represents the housing market.
         * Th main responsibility is the management of: 
         *  - avaliable units
         */
        class HousingMarket : public UnitHolder, public sim_mob::Entity {
        public:
            HousingMarket();
            virtual ~HousingMarket();

            /**
             * Inherited from Entity
             */
            virtual UpdateStatus update(timeslice now);

        protected:

            /**
             * Inherited from UnitHolder.
             */
            bool add(Unit* unit, bool reOwner);
            Unit* remove(UnitId id, bool reOwner);

            /**
             * Inherited from Entity
             */
            virtual bool isNonspatial();
            virtual void buildSubscriptionList(std::vector<sim_mob::BufferedBase*>& subsList);
            
        private:
            /**
             * Initializes the market.
             */
            void setup();
            
        private:
            volatile bool firstTime;
        };
    }
}

