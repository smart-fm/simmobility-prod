/* 
 * Copyright Singapore-MIT Alliance for Research and Technology
 * 
 * File:   UnitType.hpp
 * Author: Pedro Gandola <pedrogandola@smart.mit.edu>
 *
 * Created on 1 July, 2013, 3:04 PM
 */
#pragma once

#include "Common.hpp"
#include "Types.hpp"

namespace sim_mob {

    namespace long_term {

        class UnitType {
        public:
            UnitType(BigSerial id = INVALID_ID, std::string name = "");
            virtual ~UnitType();

            /**
             * Gets unique identifier of the Building Type.
             * @return id.
             */
            BigSerial GetId() const;

            /**
             * Gets name of the type of the building.
             * @return id.
             */
            std::string GetName() const;

            /**
             * Assign operator.
             * @param source to assign.
             * @return UnitType instance reference.
             */
            UnitType& operator=(const UnitType& source);

            /**
             * Operator to print the Building data.  
             */
            friend std::ostream& operator<<(std::ostream& strm, const UnitType& data) {
                return strm << "{"
                        << "\"id\":\"" << data.id << "\","
                        << "\"name\":\"" << data.name << "\""
                        << "}";
            }

        private:
            friend class UnitTypeDao;
        private:
            BigSerial id;
            std::string name;
        };
    }
}
