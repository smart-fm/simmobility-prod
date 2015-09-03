/*
 * UnitPrice.cpp
 *
 *  Created on: Aug 20, 2015
 *      Author: gishara
 */
#include "UnitPriceSum.hpp"

using namespace sim_mob::long_term;

UnitPriceSum::UnitPriceSum(BigSerial fmParcelId,double unitPriceSum):
		fmParcelId(fmParcelId),unitPriceSum(unitPriceSum) {}

UnitPriceSum::~UnitPriceSum() {}


BigSerial UnitPriceSum::getFmParcelId() const {
		return fmParcelId;
	}

double UnitPriceSum::getUnitPriceSum() const {
		return unitPriceSum;
	}

namespace sim_mob
{
    namespace long_term
    {
        std::ostream& operator<<(std::ostream& strm, const UnitPriceSum& data)
        {
            return strm << "{"
						<< "\"fmUnitId \":\"" << data.fmParcelId 	<< "\","
						<< "\"unitPrice \":\"" 	<< data.unitPriceSum 	<< "\","
						<< "}";
        }
    }
}






