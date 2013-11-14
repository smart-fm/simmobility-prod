/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DatabaseConfig.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 11, 2013, 11:26 AM
 */

#include "DatabaseConfig.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace sim_mob;
using namespace sim_mob::db;
using std::string;

DatabaseConfig::DatabaseConfig() 
: port(0), host(""), password(""), username(""), databaseName(""), file(""){
}
DatabaseConfig::DatabaseConfig(const string& file) 
: file(file), port(0), host(""), password(""), username(""), databaseName("") {
    if (!file.empty()) {
        boost::property_tree::ptree pt;
        boost::property_tree::read_ini(file, pt);
        this->host = pt.get<string>("database.host");
        this->port = pt.get<int>("database.port");
        this->username = pt.get<string>("database.username");
        this->password = pt.get<string>("database.password");
        this->databaseName = pt.get<string>("database.dbname");
    }
}

sim_mob::db::DatabaseConfig::DatabaseConfig(std::string& host, std::string& port, std::string& dbname)
: port(std::atoi(port.c_str())), host(host), databaseName(dbname), username(""), password(""), file("") {
}

DatabaseConfig::DatabaseConfig(const DatabaseConfig& orig) {
    this->host = orig.host;
    this->port = orig.port;
    this->username = orig.username;
    this->password = orig.password;
    this->databaseName = orig.databaseName;
    this->file = file;
}

DatabaseConfig::~DatabaseConfig() {
}

std::string DatabaseConfig::GetDatabaseName() const {
    return databaseName;
}

std::string DatabaseConfig::GetPassword() const {
    return password;
}

std::string DatabaseConfig::GetUsername() const {
    return username;
}

int DatabaseConfig::GetPort() const {
    return port;
}

std::string DatabaseConfig::GetHost() const {
    return host;
}

void DatabaseConfig::SetDatabaseName(std::string databaseName) {
    this->databaseName = databaseName;
}

void DatabaseConfig::SetPassword(std::string password) {
    this->password = password;
}

void DatabaseConfig::SetUsername(std::string username) {
    this->username = username;
}

void DatabaseConfig::SetPort(int port) {
    this->port = port;
}

void DatabaseConfig::SetHost(std::string host) {
    this->host = host;
}
