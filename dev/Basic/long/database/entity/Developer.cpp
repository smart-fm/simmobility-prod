//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/* 
 * File:   Developer.cpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 * 
 * Created on Mar 5, 2014, 5:54 PM
 */

#include "Developer.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

Developer::Developer(BigSerial id, const std::string& name,
        const std::string& type) : id(id), name(name), type(type) {
}

Developer::~Developer() {
}

BigSerial Developer::getId() const {
    return id;
}

const std::string& Developer::getName() const {
    return name;
}

const std::string& Developer::getType() const {
    return type;
}

void Developer::setName(const std::string& name) {
    this->name = name;
}

void Developer::setType(const std::string& type) {
    this->type = type;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const Developer& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"name\":\"" << data.name << "\","
                    << "\"type\":\"" << data.type << "\""
                    << "}";
        }
    }
}