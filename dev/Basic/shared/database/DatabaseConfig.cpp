/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DatabaseConfig.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 11, 2013, 11:26 AM
 */

#include "DatabaseConfig.hpp"
#include "entities/profile/ProfileBuilder.hpp"

using namespace sim_mob;
using namespace sim_mob::db;
using std::string;
namespace {
    const string SECTION = "database";
    const string PROP_HOST = "host";
    const string PROP_PASS = "password";
    const string PROP_USER = "username";
    const string PROP_DB = "dbname";
    const string PROP_PORT = "port";
}

DatabaseConfig::DatabaseConfig(const string& file)
: PropertyLoader(file, SECTION),
port(0), host(""), password(""), username(""), databaseName("") {
}

DatabaseConfig::DatabaseConfig(const DatabaseConfig& orig) 
: PropertyLoader(orig) {
    host = orig.host;
    port = orig.port;
    username = orig.username;
    password = orig.password;
    databaseName = orig.databaseName;
}

DatabaseConfig::~DatabaseConfig() {
}

const string& DatabaseConfig::getDatabaseName() const {
    return databaseName;
}

const string& DatabaseConfig::getPassword() const {
    return password;
}

const string& DatabaseConfig::getUsername() const {
    return username;
}

const string& DatabaseConfig::getHost() const {
    return host;
}

unsigned int DatabaseConfig::getPort() const {
    return port;
}

void DatabaseConfig::setDatabaseName(const string& databaseName) {
    this->databaseName = databaseName;
}

void DatabaseConfig::setPassword(const string& password) {
    this->password = password;
}

void DatabaseConfig::setUsername(const string& username) {
    this->username = username;
}

void DatabaseConfig::setPort(unsigned int port) {
    this->port = port;
}

void DatabaseConfig::setHost(const string& host) {
    this->host = host;
}

void DatabaseConfig::loadImpl(const boost::property_tree::ptree& tree) {
    host = tree.get<string>(toProp(getSection(), PROP_HOST));
    this->port = tree.get<unsigned int>(toProp(getSection(), PROP_PORT));
    this->username = tree.get<string>(toProp(getSection(), PROP_USER));
    this->password = tree.get<string>(toProp(getSection(), PROP_PASS));
    this->databaseName = tree.get<string>(toProp(getSection(), PROP_DB));
}