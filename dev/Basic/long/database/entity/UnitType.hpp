/*
 * unitType.hpp
 *
 *  Created on: Nov 24, 2014
 *      Author: gishara
 */

#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class UnitType {
        public:
            UnitType(BigSerial id = INVALID_ID,std::string name = EMPTY_STR,
                     double typicalArea = 0.0, double constructionCostPerUnit = 0.0,double demolitionCostPerUnit = 0.0, double minLosize = 0.0, int dwellingType = 0, double wtpOffset = 0.0);

            virtual ~UnitType();

            /**
             * Getters and Setters
             */
            BigSerial getId() const;
            std::string getName() const;
            double getTypicalArea() const;
            double getConstructionCostPerUnit() const;
            double getDemolitionCostPerUnit() const;
            double getMinLosize() const ;

            void setWtpOffset(double value);
            double getWtpOffset() const;


            /**
             * Operator to print the TemplateUnitType data.
             */
            friend std::ostream& operator<<(std::ostream& strm,
                    const UnitType& data);


        private:
            friend class UnitTypeDao;
        private:
            BigSerial id;
            std::string name;
            double typicalArea;
            double constructionCostPerUnit;
            double demolitionCostPerUnit;
            double minLosize;
            int	   dwellingType;
            double wtpOffset;
        };
    }
}
