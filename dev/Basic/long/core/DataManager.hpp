/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DataManager.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on Feb 11, 2011, 1:32 PM
 */
#pragma once

#include <vector>
#include <boost/unordered_map.hpp>
#include "util/SingletonHolder.hpp"
#include "database/entity/Postcode.hpp"
#include "database/entity/PostcodeAmenities.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Singleton responsible for managing necessary data for the simulation.
         */
        class DataManager {
        public:
            virtual ~DataManager();
        
            /**
             * Loads the necessary data from datasource.
             */
            void load();
        
        private:
            template<typename T> friend class DataManagerLifeCycle;
            DataManager();
            
        public:
           
            typedef std::vector<Postcode> PostcodeList;
            typedef std::vector<PostcodeAmenities> PostcodeAmenitiesList;
            typedef boost::unordered_map<BigSerial, Postcode*> PostcodeMap;
            typedef boost::unordered_map<BigSerial, PostcodeAmenities*> PostcodeAmenitiesMap;
        private:
            PostcodeList postcodes;
            PostcodeAmenitiesList amenities;
            PostcodeMap postcodesById;
            PostcodeAmenitiesMap amenitiesById;
        };
        
        /**
         * DataManager Singleton.
         */
        template<typename T = DataManager>
        struct DataManagerLifeCycle {

            static T * create() {
                T* obj = new T();
                return obj;
            }

            static void destroy(T* obj) {
                delete obj;
            }
        };
        
        typedef SingletonHolder<DataManager, DataManagerLifeCycle> DataManagerSingleton;
    }
}