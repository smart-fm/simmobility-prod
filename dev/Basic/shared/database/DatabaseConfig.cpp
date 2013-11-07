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

DB_Config::DB_Config(const string& file)
: PropertyLoader(file, SECTION),
port(0), host(""), password(""), username(""), databaseName("") {
}

DB_Config::DB_Config(const DB_Config& orig) 
: PropertyLoader(orig) {
    host = orig.host;
    port = orig.port;
    username = orig.username;
    password = orig.password;
    databaseName = orig.databaseName;
}

DB_Config::~DB_Config() {
}

const string& DB_Config::getDatabaseName() const {
    return databaseName;
}

const string& DB_Config::getPassword() const {
    return password;
}

const string& DB_Config::getUsername() const {
    return username;
}

const string& DB_Config::getHost() const {
    return host;
}

unsigned int DB_Config::getPort() const {
    return port;
}

void DB_Config::setDatabaseName(const string& databaseName) {
    this->databaseName = databaseName;
}

void DB_Config::setPassword(const string& password) {
    this->password = password;
}

void DB_Config::setUsername(const string& username) {
    this->username = username;
}

void DB_Config::setPort(unsigned int port) {
    this->port = port;
}

void DB_Config::setHost(const string& host) {
    this->host = host;
}

void DB_Config::loadImpl(const boost::property_tree::ptree& tree) {
    host = tree.get<string>(toProp(getSection(), PROP_HOST));
    this->port = tree.get<unsigned int>(toProp(getSection(), PROP_PORT));
    this->username = tree.get<string>(toProp(getSection(), PROP_USER));
    this->password = tree.get<string>(toProp(getSection(), PROP_PASS));
    this->databaseName = tree.get<string>(toProp(getSection(), PROP_DB));
}