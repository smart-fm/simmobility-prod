//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * File:   MacroEconomics.cpp
 * Author: Gishara Premarathne <gishara@smart.mit.edu>
 *
 * Created on Mar 5, 2014, 5:54 PM
 */

#include "MacroEconomics.hpp"
#include "util/Utils.hpp"

using namespace sim_mob::long_term;

HPIValues::HPIValues(double HPI1,double HPI2,double HPI3,double HPI4,double HPI5): HPI1(HPI1),HPI2(HPI2),HPI3(HPI3),HPI4(HPI4),HPI5(HPI5) {

}

HPIValues::~HPIValues() {
}

MacroEconomics::MacroEconomics(std::tm exDate, BigSerial exFactorId,
		double exFactorValue) : exDate(exDate), exFactorId(exFactorId), exFactorValue(exFactorValue) {
}

MacroEconomics::~MacroEconomics() {
}

BigSerial MacroEconomics::getExFactorId() const {
    return this->exFactorId;
}

double MacroEconomics::getExFactorValue() const {
    return this->exFactorValue;
}

namespace sim_mob {
    namespace long_term {

        std::ostream& operator<<(std::ostream& strm, const MacroEconomics& data) {
            return strm << "{"
                    << "\"exDate\":\"" << data.exDate.tm_year << data.exDate.tm_mon << data.exDate.tm_mday <<"\","
                    << "\"exFactorId\":\"" << data.exFactorId<< "\","
                    << "\"exFactorValue\":\"" << data.exFactorValue << "\""
                    << "}";
        }
    }
}


