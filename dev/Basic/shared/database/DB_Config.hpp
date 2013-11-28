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
#include "conf/PropertyLoader.hpp"

namespace sim_mob {

    namespace db {

        /**
         * Reads database properties from ini file. Fields: 
         * 
         * [database]
         * host=<host>
         * port=<port>
         * username=<db user>
         * password=<db pass>
         * dbname=<db name>
         * 
         * @param file to read the properties.
         */
        class DB_Config : public PropertyLoader{
        public:
            DB_Config(const std::string& file);
            DB_Config(const DB_Config& orig);
            DB_Config(std::string& host, std::string& port, std::string& dbname, std::string& username, std::string& password);
            virtual ~DB_Config();
            const std::string& getDatabaseName() const;
            const std::string& getPassword() const;
            const std::string& getUsername() const;
            const std::string& getHost() const;
            unsigned int getPort() const;
            void setDatabaseName(const std::string& databaseName);
            void setPassword(const std::string& password);
            void setUsername(const std::string& username);
            void setPort(unsigned int port);
            void setHost(const std::string& host);
        protected:
            /**
             * Inherited from PropertyLoader
             */
            virtual void loadImpl(const boost::property_tree::ptree& tree);
        private:
            std::string host;
            unsigned int port;
            std::string username;
            std::string password;
            std::string databaseName;
        };
    }
}

