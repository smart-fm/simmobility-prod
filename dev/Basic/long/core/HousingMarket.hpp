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

#include <boost/unordered_map.hpp>
#include "util/UnitHolder.hpp"
#include "entities/Entity.hpp"
#include "buffering/Buffered.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Represents the housing market.
         * Th main responsibility is the management of: 
         *  - avaliable units
         */
        class HousingMarket : public UnitHolder, public sim_mob::Entity {
        public:

            class UnitEntry {
            public:
                UnitEntry(const Unit& unit, double askingPrice, double hedonicPrice);
                virtual ~UnitEntry();
                const Unit& getUnit() const;
                double getAskingPrice() const;
                double getHedonicPrice() const;
                void setAskingPrice(double askingPrice);
                void setHedonicPrice(double hedonicPrice);

            private:
                const Unit& unit;
                double askingPrice;
                double hedonicPrice;
            };
            typedef boost::unordered_map<BigSerial, HousingMarket::UnitEntry> EntryMap;
        public:
            HousingMarket();
            virtual ~HousingMarket();

            void addNewEntry(const Unit& unit, double askingPrice, 
                double hedonicPrice);
            
            void removeEntry(const BigSerial& unitId);
            
            const EntryMap& getAvailableEntries();
            
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
            virtual void HandleMessage(messaging::Message::MessageType type, 
                    const messaging::Message& message);

        private:
            /**
             * Inherited from Entity
             */
            void onWorkerEnter();
            void onWorkerExit();
            
        private:
            EntryMap entriesById;
        };
    }
}

