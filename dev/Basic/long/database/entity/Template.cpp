//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Template.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 11, 2014, 5:54 PM
 */

#include "Template.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

Template::Template(BigSerial id, const std::string& name)
: id(id), name(name) {
}

Template::~Template() {
}

BigSerial Template::getId() const {
    return id;
}

const std::string& Template::getName() const {
    return name;
}

void Template::setName(const std::string& name) {
    this->name = name;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Template& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"name\":\"" << data.name << "\""
                    << "}";
        }
    }
}