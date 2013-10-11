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

namespace sim_mob {
    
    namespace db {

        class DatabaseConfig {
        public:
            DatabaseConfig(const std::string& file);
            DatabaseConfig(const DatabaseConfig& orig);
            virtual ~DatabaseConfig();

            std::string GetDatabaseName() const;
            std::string GetPassword() const;
            std::string GetUsername() const;
            int GetPort() const;
            std::string GetHost() const;
        private:
            std::string file;
            std::string host;
            int port;
            std::string username;
            std::string password;
            std::string databaseName;
        };
    }
}

