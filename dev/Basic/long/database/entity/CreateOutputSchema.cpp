//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * CreateOutputSchema.cpp
 *
 *  Created on: Nov 23, 2015
 *      Author: gishara
 */

#include "CreateOutputSchema.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

CreateOutputSchema::CreateOutputSchema(BigSerial id, const std::string& tableName,
        const std::string& query) : id(id), tableName(tableName), query(query) {
}

CreateOutputSchema::~CreateOutputSchema() {
}

BigSerial CreateOutputSchema::getId() const {
    return id;
}

const std::string& CreateOutputSchema::getTableName() const {
    return tableName;
}

const std::string& CreateOutputSchema::getQuery() const {
    return query;
}

void CreateOutputSchema::setTableName(const std::string& name) {
    this->tableName = name;
}

void CreateOutputSchema::setQuery(const std::string& queryStr) {
    this->query = queryStr;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const CreateOutputSchema& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id << "\","
                    << "\"tableName\":\"" << data.tableName << "\","
                    << "\"query\":\"" << data.query << "\""
                    << "}";
        }
    }
}


