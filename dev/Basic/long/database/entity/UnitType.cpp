/*
 * unitType.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: gishara
 */
#include "UnitType.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

UnitType::UnitType(BigSerial id,std::string name,double typicalArea, double constructionCostPerUnit)
: id(id), name(name), typicalArea(typicalArea),constructionCostPerUnit(constructionCostPerUnit){
}

UnitType::~UnitType() {
}

BigSerial UnitType::getId() const {
    return id;
}

std::string UnitType::getName() const {
    return name;
}

double UnitType::getTypicalArea() const {
    return typicalArea;
}

double UnitType::getConstructionCostPerUnit() const {
    return constructionCostPerUnit;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const UnitType& data) {
            return strm << "{"
                    << "\"id\":\"" << data.id<< "\","
                    << "\"name\":\"" << data.name << "\","
                    << "\" typicalArea\":\"" << data.typicalArea << "\""
                    << "\" constructionCostPerUnit\":\"" << data.constructionCostPerUnit << "\""
                    << "}";
        }
    }
}




