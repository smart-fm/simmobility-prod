/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   GlobalParams.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on 1 July, 2013, 3:04 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class GlobalParams {
        public:
            GlobalParams(BigSerial id = INVALID_ID, double unitTypeWeight = 0,
                    double unitAreaWeight = 0, double unitRentWeight = 0,
                    double unitStoreyWeight = 0);
            virtual ~GlobalParams();

            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            BigSerial GetId() const;

            /**
             * @return unit type weight.
             */
            double GetUnitTypeWeight() const;

            /**
             * @return unit rent weight.
             */
            double GetUnitRentWeight() const;

            /**
             * @return unit area weight.
             */
            double GetUnitAreaWeight() const;

            /**
             * @return unit storey weight.
             */
            double GetUnitStoreyWeight() const;

            /**
             * Assign operator.
             * @param source to assign.
             * @return GlobalParams instance reference.
             */
            GlobalParams& operator=(const GlobalParams& source);

            /**
             * Operator to print the Building data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const GlobalParams& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"unitTypeWeight\":\"" << data.unitTypeWeight << "\","
                        << "\"unitAreaWeight\":\"" << data.unitAreaWeight << "\","
                        << "\"unitRentWeight\":\"" << data.unitRentWeight << "\","
                        << "\"unitStoreyWeight\":\"" << data.unitStoreyWeight << "\""
                        << "}";
            }

        private:
            friend class GlobalParamsDao;
        private:
            BigSerial id;
            double unitTypeWeight;
            double unitAreaWeight;
            double unitRentWeight;
            double unitStoreyWeight;
        };
    }
}
