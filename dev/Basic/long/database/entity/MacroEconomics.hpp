//Copyright (c) 2013 Singapore-MIT Alliance for Research and Technology
//Licensed under the terms of the MIT License, as described in the file:
//   license.txt   (http://opensource.org/licenses/MIT)

/*
 * MacroEconomics.hpp
 *
 *  Created on: Jan 14, 2015
 *      Author: gishara
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

    /**
     * Represents HPI values of a building type.
     */
    class HPIValues {
    public:
    	HPIValues(double HPI1 = 0,double HPI2 = 0, double HPI3 = 0,double HPI4 = 0,double HPI5 = 0);
    	virtual ~HPIValues();

	friend std::ostream& operator<<(std::ostream& strm, const HPIValues& data);

	double getHpi1() const {
		return HPI1;
	}

	double getHpi2() const {
		return HPI2;
	}

	double getHpi3() const {
		return HPI3;
	}

	double getHpi4() const {
		return HPI4;
	}

	double getHpi5() const {
		return HPI5;
	}

    private:
		double HPI1;
		double HPI2;
		double HPI3;
		double HPI4;
		double HPI5;
};
        class MacroEconomics {
        public:
        	MacroEconomics(std::tm exDate = std::tm(),
        			BigSerial exFactorId = INVALID_ID,
        			double exFactorValue = 0);

            virtual ~MacroEconomics();

            /**
             * Getters
             */
            BigSerial getExFactorId() const;
            double getExFactorValue() const;

            /**
             * Operator to print the Developer data.
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const MacroEconomics& data);
        private:
            friend class MacroEconomicsDao;
        private:
            std::tm exDate;
            BigSerial exFactorId;
            double exFactorValue;
        };
    }
}

