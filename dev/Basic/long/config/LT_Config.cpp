/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   DatabaseConfig.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on October 11, 2013, 11:26 AM
 */

#include "LT_Config.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

using namespace sim_mob;
using namespace sim_mob::long_term;
using std::string;

PropertyLoader::PropertyLoader(const string& file, const string& section)
: file(file), section(section) {
}

PropertyLoader::PropertyLoader(const PropertyLoader& source)
: file(source.file), section(source.section) {
}

PropertyLoader::~PropertyLoader() {
}

void PropertyLoader::load() {
    if (!file.empty() && !section.empty()) {
        boost::property_tree::ptree pt;
        boost::property_tree::read_ini(file, pt);
        loadImpl(pt);
    }
}

/**
 * InjectorConfig
 */
InjectorConfig::InjectorConfig(const string& file)
: PropertyLoader(file, "events-injector"), eventsFile("") {
}

InjectorConfig::InjectorConfig(const InjectorConfig& orig)
: PropertyLoader(orig), eventsFile(orig.eventsFile) {
}

InjectorConfig::~InjectorConfig() {
}

const string& InjectorConfig::getEventsFile() const {
    return eventsFile;
}

void InjectorConfig::setEventsFile(const std::string& eventsFile) {
    this->eventsFile = eventsFile;
}

void InjectorConfig::loadImpl(const boost::property_tree::ptree& tree) {
    this->eventsFile = tree.get<string>(section + ".eventsFile");
}

/**
 * HM_Config
 */
HM_Config::HM_Config(const string& file)
: PropertyLoader(file, "housing-market"), timeOnMarket(0) {
}

HM_Config::HM_Config(const HM_Config& orig)
: PropertyLoader(orig), timeOnMarket(orig.timeOnMarket) {
}

HM_Config::~HM_Config() {
}

unsigned int HM_Config::getTimeOnMarket() const {
    return timeOnMarket;
}

void HM_Config::setTimeOnMarket(unsigned int timeOnMarket) {
    this->timeOnMarket = timeOnMarket;
}

void HM_Config::loadImpl(const boost::property_tree::ptree& tree) {
    this->timeOnMarket = tree.get<unsigned int>(section + ".timeOnMarket");
}

/**
 * LT_Config
 */
LT_Config::LT_Config(const std::string& file)
: PropertyLoader(file, "global"), hmConfig(file), injectorConfig(file) {
}

LT_Config::LT_Config(const LT_Config& orig)
: PropertyLoader(orig), hmConfig(orig.hmConfig),
injectorConfig(orig.injectorConfig) {
}

LT_Config::~LT_Config() {
}

const HM_Config& LT_Config::getHM_Config() const {
    return hmConfig;
}

const InjectorConfig& LT_Config::getInjectorConfig() const {
    return injectorConfig;
}

void LT_Config::loadImpl(const boost::property_tree::ptree& tree) {
    hmConfig.loadImpl(tree);
    injectorConfig.loadImpl(tree);
}