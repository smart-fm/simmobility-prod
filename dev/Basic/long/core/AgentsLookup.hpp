/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   AgentsLookup.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Feb 18, 2011, 1:32 PM
 */
#pragma once

#include <boost/unordered_map.hpp>
#include "util/SingletonHolder.hpp"
#include "agent/impl/HouseholdAgent.hpp"
#include "agent/impl/DeveloperAgent.hpp"
#include "LoggerAgent.hpp"
#include "EventsInjector.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Singleton responsible **only** for maintaining a map with pointers 
         * to all agents in the system for lookup.
         */
        class AgentsLookup {
        public:
            typedef boost::unordered_map<BigSerial, const HouseholdAgent*> HouseholdMap;
            typedef boost::unordered_map<BigSerial, const DeveloperAgent*> DeveloperMap;

        public:
            virtual ~AgentsLookup();

            /**
             * Adds new household agent for lookup.
             * @param agent pointer to add.
             */
            void addHousehold(const HouseholdAgent* agent);
            
            /**
             * Adds new developer agent for lookup.
             * @param agent pointer to add.
             */
            void addDeveloper(const DeveloperAgent* agent);
            
            /**
             * Gets logger agent.
             */
            LoggerAgent& getLogger();
            
            /**
             * Gets agent responsible to inject external events into the simulation.
             */
            EventsInjector& getEventsInjector();
            
            /**
             * Clears all data.
             */
            void reset();

            /**
             * Getters 
             */
            const HouseholdAgent* getHouseholdById(const BigSerial id) const;
            const DeveloperAgent* getDeveloperById(const BigSerial id) const;
        private:
            template<typename T> friend class AgentsLookupLifeCycle;
            AgentsLookup();

        private:
            HouseholdMap householdsById;
            DeveloperMap developersById;
            LoggerAgent logger;
            EventsInjector injector;
        };

        /**
         * AgentsLookup Singleton.
         */
        template<typename T = AgentsLookup>
        struct AgentsLookupLifeCycle {

            static T * create() {
                T* obj = new T();
                return obj;
            }

            static void destroy(T* obj) {
                delete obj;
            }
        };

        typedef SingletonHolder<AgentsLookup, AgentsLookupLifeCycle> AgentsLookupSingleton;
    }
}