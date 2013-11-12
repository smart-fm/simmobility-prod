/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   LT_Config.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on November 6, 2013, 11:26 AM
 */
#pragma once
#include <string>
#include "Common.hpp"
#include "conf/PropertyLoader.hpp"
#include "util/SingletonHolder.hpp"

namespace sim_mob {

    namespace long_term {

        /**
         * Reads long-term properties from ini file. 
         *
         * Supported fields:
         * [events injector]
         * events_file=<file absolute path>
         * 
         */
        class InjectorConfig : public PropertyLoader {
        public:
            /**
             * Constructor to load from given file.
             * @param file to load properties.
             */
            InjectorConfig(const std::string& file);
            InjectorConfig(const InjectorConfig& orig);
            virtual ~InjectorConfig();
            const std::string& getEventsFile() const;
            void setEventsFile(const std::string& filePath);

        protected:
            friend class LT_Config;
            /**
             * Inherited from PropertyLoader
             */
            void loadImpl(const boost::property_tree::ptree& tree);
        private:
            std::string eventsFile;
        };

        /**
         * Reads long-term properties from ini file. 
         *
         * [housing market]
         * time_on_market=<int 0-365>
         */
        class HM_Config : public PropertyLoader {
        public:
            /**
             * Constructor to load from given file.
             * @param file to load properties.
             */
            HM_Config(const std::string& file);
            HM_Config(const HM_Config& orig);
            virtual ~HM_Config();

            unsigned int getTimeOnMarket() const;
            void setTimeOnMarket(unsigned int filePath);

        protected:
            friend class LT_Config;
            /**
             * Inherited from PropertyLoader
             */
            void loadImpl(const boost::property_tree::ptree& tree);
        private:
            unsigned int timeOnMarket;
        };

        /**
         * Reads long-term properties from ini file.
         * 
         * Containing all configurations.
         *
         * It is possible to avoid the configuration. 
         */
        class LT_Config : public PropertyLoader {
        public:
            LT_Config(const std::string& file);
            LT_Config(const LT_Config& orig);
            virtual ~LT_Config();

            const HM_Config& getHM_Config() const;
            const InjectorConfig& getInjectorConfig() const;
        protected:
            /**
             * Inherited from PropertyLoader
             */
            void loadImpl(const boost::property_tree::ptree& tree);
        private:
            HM_Config hmConfig;
            InjectorConfig injectorConfig;
        };

        /**
         * Configuration Singleton.
         */
        template<typename T = LT_Config>
        struct ConfigLifeCycle {

            static T * create() {
                T* config = new T(LT_CONFIG_FILE);
                config->load();
                return config;
            }

            static void destroy(T* config) {
                delete config;
            }
        };

        typedef SingletonHolder<LT_Config, ConfigLifeCycle> LT_ConfigSingleton;
    }
}

