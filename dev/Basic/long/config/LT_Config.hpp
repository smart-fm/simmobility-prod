/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DatabaseConfig.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on October 11, 2013, 11:26 AM
 */
#pragma once
#include <string>
#include <boost/property_tree/ptree.hpp>

namespace sim_mob {

    namespace long_term {

        /**
         * Represents an abstract implementation properties file loader.
         */
        class PropertyLoader {
        public:
            /**
             * @param file to read the properties.
             * @param section prefix of the property.
             */
            PropertyLoader(const std::string& file, const std::string& section);
            PropertyLoader(const PropertyLoader& source);
            virtual ~PropertyLoader();

            /**
             * Loads properties.
             */
            void load();
        protected:
            /**
             * Method to be implemented by specific loader.
             * @param tree to load.
             */
            virtual void loadImpl(const boost::property_tree::ptree& tree) = 0;
        protected:
            std::string section;
        private:
            std::string file;
        };

        /**
         * Reads long-term properties from ini file. 
         *
         * Supported fields:
         * [events-injector]
         * eventsFile=<file absolute path>
         * 
         * @param file to read the properties.
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
         * [housing-market]
         * time-on-market=<int 0-365>
         * @param file to read the properties.
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
         * @param file to read the properties.
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
    }
}

