/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   PropertyLoader.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on November 6, 2013, 10:40 AM
 */

#include "PropertyLoader.hpp"
#include <boost/property_tree/ini_parser.hpp>
using namespace sim_mob;
using std::string;

namespace {
    const string PROP_SEPARATOR = ".";
}

PropertyLoader::PropertyLoader(const string& filePath, const string& section)
: filePath(filePath), section(section) {
}

PropertyLoader::PropertyLoader(const PropertyLoader& source)
: filePath(source.filePath), section(source.section) {
}

PropertyLoader::~PropertyLoader() {
}

void PropertyLoader::load() {
    if (!filePath.empty() && !section.empty()) {
        boost::property_tree::ptree pt;
        boost::property_tree::read_ini(filePath, pt);
        loadImpl(pt);
    }
}

const string& PropertyLoader::getSection() const {
    return section;
}

const string& PropertyLoader::getFilePath() const {
    return filePath;
}

const string PropertyLoader::toProp(const string& section,
        const string& prop) {
    return (section + PROP_SEPARATOR + prop);
}