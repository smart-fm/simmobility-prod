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


using namespace sim_mob;
using namespace sim_mob::long_term;
using std::string;

namespace {

    //sections
    const string SECTION_GLOBAL = "global";
    const string SECTION_EVENTS_INJECTOR = "events injector";
    const string SECTION_HOUSING_MARKET = "housing market";
    //properties
    const string PROP_EVENTS_FILE = "events_file";
    const string PROP_TIME_ON_MARKET = "time_on_market";
}

/**
 * InjectorConfig
 */
InjectorConfig::InjectorConfig(const string& file)
: PropertyLoader(file, SECTION_EVENTS_INJECTOR), eventsFile("") {
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
    this->eventsFile = tree.get<string>(toProp(getSection(), PROP_EVENTS_FILE));
}

/**
 * HM_Config
 */
HM_Config::HM_Config(const string& file)
: PropertyLoader(file, SECTION_HOUSING_MARKET), timeOnMarket(0) {
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
    this->timeOnMarket = tree.get<unsigned int>(toProp(getSection(), PROP_TIME_ON_MARKET));
}

/**
 * LT_Config
 */
LT_Config::LT_Config(const std::string& file)
: PropertyLoader(file, SECTION_GLOBAL), hmConfig(file), injectorConfig(file) {
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